#!/usr/bin/env python3

from __future__ import annotations

import argparse
import json
import os
import platform
import shutil
import subprocess
import sys
import threading
from concurrent.futures import ThreadPoolExecutor, as_completed
from datetime import datetime
from pathlib import Path

from config import BuildMatrixConfig
from logger import Logger
from matrix import BuildCase, generate_build_matrix
from models import CaseResult
from project_config import load_project_config
from report import write_case_json, write_summary_json
from runner import run_case, select_generator, safe_rmtree


PROGRAM_NAME = "build-matrix"


def parse_args() -> argparse.Namespace:

    parser = argparse.ArgumentParser(
        prog=PROGRAM_NAME,
        description="Run the full CMake build matrix for a project.",
    )

    parser.add_argument(
        "--source",
        type=Path,
        default=Path.cwd(),
        help="Project root.",
    )

    parser.add_argument(
        "--output",
        type=Path,
        default=Path("build-matrix-report"),
        help="Output directory.",
    )

    parser.add_argument(
        "--project-config",
        type=Path,
        default=None,
        help=(
            "Path to a project.json describing the project's package name, "
            "CMake options, etc. Defaults to tools/build_matrix/project.json. Use "
            "to reuse these tools in a different CMake project."
        ),
    )

    parser.add_argument(
        "--resume",
        action="store_true",
        help="Resume an interrupted run.",
    )

    parser.add_argument(
        "--only",
        metavar="CASE",
        help="Run only a single case.",
    )

    parser.add_argument(
        "--jobs",
        type=int,
        default=None,
        help="Parallel job count within a single case's build (--parallel).",
    )

    parser.add_argument(
        "--workers",
        type=int,
        default=min(4, os.cpu_count() or 1),
        help=(
            "Number of cases to build concurrently. Defaults to "
            "min(4, cpu count). Use --workers 1 to build cases serially."
        ),
    )

    parser.add_argument(
        "--verbose",
        action="store_true",
    )

    return parser.parse_args()


def prepare_output(
    config: BuildMatrixConfig,
) -> None:

    config.ensure_directories()


def copy_html(
    config: BuildMatrixConfig,
) -> None:

    shutil.copytree(
        config.html_directory,
        config.html,
        dirs_exist_ok=True,
    )


def load_completed_cases(
    config: BuildMatrixConfig,
) -> set[str]:

    if not config.summary_json.exists():
        return set()

    try:

        summary = json.loads(
            config.summary_json.read_text(
                encoding="utf-8",
            )
        )

    except Exception:

        return set()

    completed: set[str] = set()

    for case in summary.get(
        "cases",
        [],
    ):

        if case.get(
            "completed",
            False,
        ):

            completed.add(
                case["name"]
            )

    return completed


def write_metadata(
    config: BuildMatrixConfig,
    results,
) -> None:

    metadata = {
        "generated": datetime.now().isoformat(),
        "platform": platform.platform(),
        "python": platform.python_version(),
        "generator": config.generator or "(default)",
        "workers": config.workers,
        "cases": len(results),
    }

    try:

        metadata["cmake"] = subprocess.run(
            [
                "cmake",
                "--version",
            ],
            capture_output=True,
            text=True,
            check=False,
        ).stdout.splitlines()[0]

    except Exception:

        metadata["cmake"] = "Unknown"

    (config.output / "metadata.json").write_text(
        json.dumps(
            metadata,
            indent=2,
        ),
        encoding="utf-8",
    )


def filter_cases(
    matrix: list[BuildCase],
    name: str | None,
) -> list[BuildCase]:

    if name is None:
        return matrix

    cases = [
        case
        for case in matrix
        if case.name == name
    ]

    if not cases:

        raise ValueError(
            f"Unknown case: {name}"
        )

    return cases


def run_case_safely(
    case: BuildCase,
    config: BuildMatrixConfig,
    project,
    logger: Logger,
):
    """
    Runs one case and guarantees a CaseResult comes back no matter what -
    an unexpected exception here must not take the rest of the matrix down
    with it.
    """

    build_directory = config.build_directory(case.name)

    try:

        safe_rmtree(build_directory)

        build_directory.mkdir(
            parents=True,
            exist_ok=True,
        )

        return run_case(
            case=case,
            build_dir=build_directory,
            config=config,
            project=project,
            logger=logger,
        )

    except Exception as exc:

        logger.info(
            f"[{case.name}] CRASHED: {type(exc).__name__}: {exc}"
        )

        result = CaseResult(name=case.name)
        result.success = False
        result.error = f"{type(exc).__name__}: {exc}"

        return result


def main() -> int:

    args = parse_args()

    config = BuildMatrixConfig(
        source=args.source.resolve(),
        output=args.output.resolve(),
        jobs=args.jobs,
        workers=max(1, args.workers),
        verbose=args.verbose,
        generator=select_generator(),
    )

    # Avoid oversubscribing the machine: if the caller didn't pin down a
    # specific per-build job count, split the available cores across the
    # concurrent workers instead of letting every case ask for "all cores".
    if config.jobs is None and config.workers > 1:

        cpu_count = os.cpu_count() or config.workers
        config.jobs = max(1, cpu_count // config.workers)

    project = load_project_config(args.project_config)

    prepare_output(config)
    copy_html(config)

    logger = Logger(config)

    matrix = generate_build_matrix(project)
    matrix = filter_cases(
        matrix,
        args.only,
    )

    completed: set[str] = set()

    if args.resume:
        completed = load_completed_cases(
            config
        )

    pending = [
        case
        for case in matrix
        if case.name not in completed
    ]

    for case in matrix:

        if case.name in completed:

            logger.info(
                f"Skipping completed case: {case.name}"
            )

    if config.generator:
        logger.info(f"Using generator: {config.generator}")

    logger.info(
        f"Running {len(pending)} case(s) with {config.workers} worker(s)"
        + (
            f" ({config.jobs} build job(s) each)"
            if config.jobs is not None
            else ""
        )
    )

    results = []
    results_lock = threading.Lock()

    def handle_result(result) -> None:

        with results_lock:

            results.append(result)

            write_case_json(
                config.case_json(result.name),
                result,
            )

            write_summary_json(
                config.summary_json,
                results,
            )

        status = "PASS" if result.success else "FAIL"

        logger.info("")
        logger.info("=" * 80)
        logger.info(f"[{status}] {result.name}")
        logger.info("=" * 80)

    try:

        if config.workers <= 1:

            for case in pending:
                handle_result(
                    run_case_safely(case, config, project, logger)
                )

        else:

            with ThreadPoolExecutor(
                max_workers=config.workers
            ) as executor:

                futures = {
                    executor.submit(
                        run_case_safely,
                        case,
                        config,
                        project,
                        logger,
                    ): case
                    for case in pending
                }

                for future in as_completed(futures):
                    handle_result(future.result())

    except KeyboardInterrupt:

        logger.info("")
        logger.info("Interrupted. Partial results have been saved.")
        logger.info("Re-run with --resume to continue.")

        return 130

    write_metadata(
        config,
        results,
    )

    passed = sum(
        1
        for result in results
        if result.success
    )

    failed = (
        len(results)
        - passed
    )

    print()
    print("=" * 80)
    print("BUILD MATRIX COMPLETE")
    print("=" * 80)
    print()

    print(
        f"Passed : {passed}"
    )

    print(
        f"Failed : {failed}"
    )

    print(
        f"Total  : {len(results)}"
    )

    print()

    return (
        0
        if failed == 0
        else 1
    )


if __name__ == "__main__":
    sys.exit(
        main()
    )