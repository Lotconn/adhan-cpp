from __future__ import annotations

import os
import shutil
import time
from pathlib import Path

from discover import discover_binaries, discover_examples, find_executable
from install_test import create_consumer_project
from models import CaseResult, ExampleResult
from logger import Logger
from matrix import LibraryType


def select_generator() -> str | None:
    """
    Prefer Ninja when it's available: it's a single-config generator (so
    build output always lands directly under the build directory, with no
    ambiguity about Debug/Release subfolders) and it configures + builds
    noticeably faster than the default generator on most platforms,
    especially Visual Studio on Windows.

    Returns None to let CMake fall back to its own platform default.
    """

    if shutil.which("ninja"):
        return "Ninja"

    return None


def safe_rmtree(
    path: Path,
    attempts: int = 5,
    delay: float = 0.4,
) -> None:
    """
    Best-effort recursive delete. On Windows, a file can stay briefly
    locked after a subprocess that touched it exits (antivirus scanning,
    delayed handle release, etc.), so this retries a few times instead of
    either failing the whole case or silently ignoring the problem on the
    first try.
    """

    if not path.exists():
        return

    for attempt in range(attempts):

        try:
            shutil.rmtree(path)
            return

        except OSError:

            if attempt == attempts - 1:
                return

            time.sleep(delay)


def _parallel_args(config) -> list[str]:
    args = ["--parallel"]

    if config.jobs is not None:
        args.append(str(config.jobs))

    return args


def _configure_command(
    case,
    project,
    config,
    source: Path,
    build: Path,
) -> list[str]:

    command = ["cmake"]

    if config.generator:
        command += ["-G", config.generator]

    command += [
        "-S",
        str(source),
        "-B",
        str(build),
        f"-D{project.build_type_var}={case.build_type.value}",
        f"-D{project.shared_libs_var}="
        f"{'ON' if case.library is LibraryType.SHARED else 'OFF'}",
    ]

    for option in project.options:

        value = case.options.get(option.id, False)

        command.append(
            f"-D{option.cmake_var}={'ON' if value else 'OFF'}"
        )

    return command


def _build_command(
    build: Path,
    case,
    config,
) -> list[str]:

    # --config is meaningful for multi-config generators (e.g. the default
    # Visual Studio generator on Windows) and silently ignored by
    # single-config generators, so it's always safe to pass.
    return [
        "cmake",
        "--build",
        str(build),
        "--config",
        case.build_type.value,
        *_parallel_args(config),
    ]


def _test_command(
    build: Path,
    case,
    config,
    project,
) -> list[str]:

    return [
        "cmake",
        "--build",
        str(build),
        "--config",
        case.build_type.value,
        "--target",
        project.test_target,
        *_parallel_args(config),
    ]


def _install_command(
    build: Path,
    prefix: Path,
    case,
    config,
) -> list[str]:

    return [
        "cmake",
        "--install",
        str(build),
        "--config",
        case.build_type.value,
        "--prefix",
        str(prefix),
    ]


def _consumer_configure(
    source: Path,
    build: Path,
    config,
) -> list[str]:

    command = ["cmake"]

    if config.generator:
        command += ["-G", config.generator]

    command += [
        "-S",
        str(source),
        "-B",
        str(build),
    ]

    return command


def _consumer_build(
    build: Path,
    config,
) -> list[str]:

    return [
        "cmake",
        "--build",
        str(build),
        *_parallel_args(config),
    ]


def _run_command(
    executable: Path,
    args: list[str] | None = None,
) -> list[str]:

    return [
        str(executable),
        *(args or []),
    ]


def discover_artifacts(
    build_directory: Path,
) -> list[dict]:

    artifacts = []

    for file in discover_binaries(build_directory):

        artifacts.append(
            {
                "name": file.name,
                "path": str(file.relative_to(build_directory)),
                "size": file.stat().st_size,
            }
        )

    artifacts.sort(
        key=lambda x: x["path"]
    )

    return artifacts


def run_case(
    *,
    case,
    build_dir: Path,
    config,
    project,
    logger: Logger,
):

    logfile = config.logs / f"{case.name}.log"

    logfile.parent.mkdir(
        parents=True,
        exist_ok=True,
    )

    if logfile.exists():
        logfile.unlink()

    result = CaseResult(
        name=case.name,
        log_file=str(
            logfile.relative_to(
                config.output
            )
        ),
    )

    configure = logger.run(
        _configure_command(
            case,
            project,
            config,
            config.source,
            build_dir,
        ),
        cwd=config.source,
        logfile=logfile,
        case_name=case.name,
    )

    result.configure = configure

    if not configure.success:
        return result

    build = logger.run(
        _build_command(
            build_dir,
            case,
            config,
        ),
        cwd=config.source,
        logfile=logfile,
        case_name=case.name,
    )

    result.build = build

    if not build.success:
        return result

    if case.run_tests:

        tests = logger.run(
            _test_command(
                build_dir,
                case,
                config,
                project,
            ),
            cwd=config.source,
            logfile=logfile,
            case_name=case.name,
        )

        result.tests = tests

        if not tests.success:
            return result

    if case.install:

        install_dir = config.install_directory(case.name)
        consumer_project = config.consumer_directory(case.name)
        consumer_build = consumer_project / "build"

        try:

            safe_rmtree(install_dir)

            install = logger.run(
                _install_command(
                    build_dir,
                    install_dir,
                    case,
                    config,
                ),
                cwd=config.source,
                logfile=logfile,
                case_name=case.name,
            )

            result.install = install

            if not install.success:
                return result

            create_consumer_project(
                config.template_directory,
                consumer_project,
                install_dir,
                project.package_name,
                project.link_target,
                project.cmake_minimum_version,
                case.build_type.value,
            )

            consumer_configure = logger.run(
                _consumer_configure(
                    consumer_project,
                    consumer_build,
                    config,
                ),
                cwd=consumer_project,
                logfile=logfile,
                case_name=case.name,
            )

            result.consumer_configure = consumer_configure

            if not consumer_configure.success:
                return result

            consumer_build_result = logger.run(
                _consumer_build(
                    consumer_build,
                    config,
                ),
                cwd=consumer_project,
                logfile=logfile,
                case_name=case.name,
            )

            result.consumer_build = consumer_build_result

            if not consumer_build_result.success:
                return result

            consumer = find_executable(
                consumer_build,
                preferred_name="consumer",
            )

            env = os.environ.copy()
            env_changed = False

            if os.name == "nt":
                bin_dir = install_dir / "bin"
                env["PATH"] = os.pathsep.join([str(bin_dir), env["PATH"]])
                env_changed = True

            consumer_run = logger.run(
                _run_command(
                    consumer,
                ),
                cwd=consumer.parent,
                logfile=logfile,
                case_name=case.name,
                env=env,
                env_changed=env_changed,
            )

            result.consumer_run = consumer_run

            if not consumer_run.success:
                return result

        finally:

            safe_rmtree(consumer_project)
            safe_rmtree(install_dir)

    for executable in discover_examples(
        build_dir,
        subdirectory=project.examples_subdirectory,
    ):

        # Look the example up by stem so the config is spelled the same on
        # every platform ("adhan-cli", not "adhan-cli.exe"). Anything not
        # described in project.json gets a single bare run that must exit 0.
        spec = project.example_for(executable.stem)

        for run in spec.runs:

            command = _run_command(
                executable,
                run.args,
            )

            timeout = (
                run.timeout_seconds
                if run.timeout_seconds is not None
                else project.example_timeout_seconds
            )

            example = logger.run(
                command,
                cwd=executable.parent,
                logfile=logfile,
                case_name=case.name,
                step_name=f"{executable.stem}: {run.name}",
                expected_returncodes=run.expected_exit_codes,
                expect_stdout_contains=run.expect_stdout_contains,
                timeout=timeout,
            )

            result.examples.append(
                ExampleResult(
                    name=executable.name,
                    run_name=run.name,
                    path=str(
                        executable.relative_to(
                            build_dir
                        )
                    ),
                    args=list(run.args),
                    success=example.success,
                    returncode=example.returncode,
                    expected_returncodes=list(
                        run.expected_exit_codes
                    ),
                    missing_output=list(example.missing_output),
                    launch_error=example.launch_error,
                    elapsed=example.elapsed,
                )
            )

    result.artifacts = discover_artifacts(
        build_dir
    )

    # A case passes only if every command it ran exited with an expected
    # code AND every example did too. This is stated over the recorded
    # steps rather than inferred from "we got this far", so that adding a
    # step above without an early-return guard cannot silently widen what
    # counts as a pass.
    result.success = result.compute_success()

    return result