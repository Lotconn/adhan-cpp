from __future__ import annotations

import json
from pathlib import Path

from models import CaseResult


def _case_summary(
    result: CaseResult,
) -> dict:

    summary: dict = {
        "name": result.name,
        "success": result.success,
        # A case is only "completed" (and thus skippable by --resume) if it
        # actually ran to a real pass/fail outcome. A case aborted by an
        # unexpected exception should be retried on the next --resume.
        "completed": result.error is None,
        "error": result.error,
        "log": result.log_file,
        "artifacts": result.artifacts,
        "examples": [
            {
                "name": example.name,
                "run_name": example.run_name,
                "label": example.label,
                "args": example.args,
                "success": example.success,
                "returncode": example.returncode,
                "expected_returncodes": example.expected_returncodes,
                "missing_output": example.missing_output,
                "elapsed": example.elapsed,
            }
            for example in result.examples
        ],
    }

    if result.configure is not None:
        summary["configure"] = result.configure.to_json()

    if result.build is not None:
        summary["build"] = result.build.to_json()

    if result.tests is not None:
        summary["tests"] = result.tests.to_json()

    if result.install is not None:
        summary["install"] = result.install.to_json()

    if result.consumer_configure is not None:
        summary["consumer_configure"] = result.consumer_configure.to_json()

    if result.consumer_build is not None:
        summary["consumer_build"] = result.consumer_build.to_json()

    if result.consumer_run is not None:
        summary["consumer_run"] = result.consumer_run.to_json()

    return summary


def write_case_json(
    filename: Path,
    result: CaseResult,
) -> None:

    filename.parent.mkdir(
        parents=True,
        exist_ok=True,
    )

    filename.write_text(
        json.dumps(
            result.to_json(),
            indent=2,
            sort_keys=False,
        ),
        encoding="utf-8",
    )


def write_summary_json(
    filename: Path,
    results: list[CaseResult],
) -> None:

    filename.parent.mkdir(
        parents=True,
        exist_ok=True,
    )

    # Cases can finish out of order when run in parallel; keep the report
    # deterministic regardless of completion order.
    ordered = sorted(
        results,
        key=lambda result: result.name,
    )

    passed = sum(
        1
        for result in ordered
        if result.success
    )

    failed = len(ordered) - passed

    summary = {
        "version": 1,
        "passed": passed,
        "failed": failed,
        "total": len(ordered),
        "cases": [
            _case_summary(result)
            for result in ordered
        ],
    }

    filename.write_text(
        json.dumps(
            summary,
            indent=2,
            sort_keys=False,
        ),
        encoding="utf-8",
    )