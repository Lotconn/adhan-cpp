#!/usr/bin/env python3
"""
Build the landing page that ties several build-matrix reports together.

The sweep runs once per platform, and each run uploads its own
`build-matrix-report/` directory as a CI artifact. Downloading all of
those artifacts into one directory gives a tree like:

    site/
    |-- report-ubuntu-24.04/
    |   |-- matrix.json
    |   |-- metadata.json
    |   |-- cases/
    |   |-- logs/
    |   `-- html/index.html
    |-- report-windows-latest/
    `-- report-macos-latest/

This script runs against that directory. It fills in
templates/index.html and writes index.html + style.css at the root, so
the whole tree can be published as a static site.

Pass the platforms you expected to run with --expect. A platform whose
artifact never arrived leaves no directory behind, so without that list
it would produce no card at all and vanish from the page. Expected
platforms always get a card; ones without a readable report are marked
unavailable and rendered as plain <div> elements, so there is no link
to a page that isn't there.

Reports found on disk that were not expected also get a card, since an
unexpected artifact is worth seeing rather than hiding.

The per-platform reports are left untouched: their asset paths are
already relative, so they work unmodified from a subdirectory.

Usage:
    python3 tools/ci-specific/make_index.py site \
        --project-name adhan \
        --commit "$GITHUB_SHA" \    # or ${{ github.sha }}
        --ref "$GITHUB_REF_NAME" \  # or ${{ github.ref_name}}
        --expect ubuntu-24.04 \
        --expect windows-latest \
        --expect macos-latest

Exit codes:
    0  index written, every expected report present and green
    1  usage / input error (nothing written)
    2  index written, but some cases failed or some reports are missing
"""

from __future__ import annotations

import argparse
import html
import json
import shutil
import sys
from datetime import datetime, timezone
from pathlib import Path

TEMPLATE_DIRECTORY = Path(__file__).resolve().parent / "templates"

# Prefix CI gives each uploaded artifact, e.g. "report-macos-latest".
# Directories not matching this are ignored, so an artifact holding
# something other than a report cannot become a platform card.
REPORT_PREFIX = "report-"


class InputError(Exception):
    """A problem with the inputs that should stop the run."""


def parse_args() -> argparse.Namespace:

    parser = argparse.ArgumentParser(
        description="Generate the build-matrix landing page.",
    )

    parser.add_argument(
        "site",
        type=Path,
        help="Directory holding the downloaded per-platform reports.",
    )

    parser.add_argument(
        "--project-name",
        default="Project",
        help="Name shown in the page heading.",
    )

    parser.add_argument(
        "--commit",
        default="",
        help="Commit SHA to display, e.g. $GITHUB_SHA.",
    )

    parser.add_argument(
        "--ref",
        default="",
        help="Branch or tag to display, e.g. $GITHUB_REF_NAME.",
    )

    parser.add_argument(
        "--expect",
        action="append",
        default=[],
        metavar="PLATFORM",
        help=(
            "A platform that should have produced a report, e.g. "
            "'ubuntu-24.04'. Repeat once per platform. Expected "
            "platforms with no readable report are shown as "
            "unavailable rather than omitted."
        ),
    )

    parser.add_argument(
        "--prefix",
        default=REPORT_PREFIX,
        help=(
            "Directory-name prefix identifying a report "
            f"(default: {REPORT_PREFIX!r})."
        ),
    )

    parser.add_argument(
        "--allow-empty",
        action="store_true",
        help=(
            "Write the page even when nothing was expected and nothing "
            "was found. Off by default: an empty site usually means the "
            "artifacts did not download, and a blank page hides that."
        ),
    )

    return parser.parse_args()


def platform_label(directory_name: str, prefix: str) -> str:
    """Turn 'report-ubuntu-24.04' into 'ubuntu-24.04'."""

    if directory_name.startswith(prefix):
        return directory_name[len(prefix):] or directory_name

    return directory_name


def read_json(path: Path) -> dict:

    try:
        return json.loads(path.read_text(encoding="utf-8"))

    except FileNotFoundError:
        raise InputError(f"no {path.name}")

    except OSError as error:
        raise InputError(f"could not read {path.name} ({error})")

    except json.JSONDecodeError as error:
        raise InputError(f"{path.name} is not valid JSON ({error})")


def collect_report(
    site: Path,
    label: str,
    prefix: str,
    expected: bool,
) -> dict:
    """
    Gather everything the landing page needs about one platform.

    A platform with no directory, or with a directory that cannot be
    read, still produces a card. A run that did not happen must not look
    the same as a run that was never asked for.
    """

    directory_name = f"{prefix}{label}"
    directory = site / directory_name

    report: dict = {
        "label": label,
        "directory": directory_name,
        "expected": expected,
        "readable": False,
        "passed": 0,
        "failed": 0,
        "total": 0,
        "generated": "",
        "cmake": "",
        "platform": "",
        "error": "",
    }

    if not directory.is_dir():
        report["error"] = "no report was uploaded"
        return report

    try:
        summary = read_json(directory / "matrix.json")

    except InputError as error:
        report["error"] = str(error)
        return report

    report["readable"] = True
    report["passed"] = int(summary.get("passed", 0))
    report["failed"] = int(summary.get("failed", 0))
    report["total"] = int(summary.get("total", 0))

    # metadata.json is a nicety, not a requirement: a report without it
    # is still usable, so its absence must not downgrade the card.
    try:
        metadata = read_json(directory / "metadata.json")

    except InputError:
        return report

    report["generated"] = str(metadata.get("generated", ""))
    report["cmake"] = str(metadata.get("cmake", ""))
    report["platform"] = str(metadata.get("platform", ""))

    return report


def find_reports(
    site: Path,
    prefix: str,
    expected: list[str],
) -> list[dict]:
    """
    One entry per expected platform, plus any unexpected report found.

    Expected platforms come first, in the order given, so the page keeps
    a stable layout across runs even as individual platforms fail.
    """

    if not site.is_dir():
        raise InputError(f"Not a directory: {site}")

    found = sorted(
        platform_label(entry.name, prefix)
        for entry in site.iterdir()
        if entry.is_dir() and entry.name.startswith(prefix)
    )

    ordered: list[tuple[str, bool]] = [
        (label, True)
        for label in expected
    ]

    ordered.extend(
        (label, False)
        for label in found
        if label not in expected
    )

    return [
        collect_report(site, label, prefix, expected_flag)
        for label, expected_flag in ordered
    ]


def card_state(report: dict) -> tuple[str, str]:
    """Return the (state, status text) pair a card displays."""

    if not report["readable"]:
        return "unavailable", f"Report unavailable — {report['error']}"

    if report["failed"] > 0:
        return (
            "fail",
            f"{report['failed']} of {report['total']} cases failed",
        )

    return "pass", f"All {report['total']} cases passed"


def render_card(site: Path, report: dict) -> str:
    """
    Render one platform card.

    A card with a report is an <a>; a card without one is a <div>. A
    link that leads nowhere is worse than no link, and the difference
    should reach a screen reader too, not only the eye.
    """

    escape = html.escape

    state, status = card_state(report)

    accent = {
        "pass": "border-l-green-600",
        "fail": "border-l-red-600",
        "unavailable": "border-l-amber-500",
    }[state]

    status_colour = {
        "pass": "text-green-700",
        "fail": "text-red-700",
        "unavailable": "text-amber-700",
    }[state]

    base = f"block rounded bg-white shadow border-l-4 p-4 {accent}"

    target = f"{report['directory']}/html/index.html"

    # Only link to the HTML report if it actually arrived. A partial
    # upload should still be reachable, so fall back to the directory.
    if state != "unavailable" and not (site / target).exists():
        target = report["directory"]
        state_note = " (raw files only)"

    else:
        state_note = ""

    meta_lines = []

    if report["cmake"]:
        meta_lines.append(f"<div>{escape(report['cmake'])}</div>")

    if report["platform"]:
        meta_lines.append(
            f'<div class="break-all">{escape(report["platform"])}</div>'
        )

    if report["generated"]:
        meta_lines.append(
            f"<div>Run at {escape(report['generated'])}</div>"
        )

    if not report["expected"]:
        meta_lines.append(
            '<div class="text-amber-700">Not in the expected '
            "platform list</div>"
        )

    meta = "\n            ".join(meta_lines) or "&nbsp;"

    inner = (
        '          <div class="font-semibold text-lg font-mono '
        f'break-all">{escape(report["label"])}</div>\n'
        f'          <div class="mt-1 text-sm font-semibold '
        f'{status_colour}">{escape(status + state_note)}</div>\n'
        '          <div class="mt-2 text-xs text-gray-500">\n'
        f"            {meta}\n"
        "          </div>"
    )

    if state == "unavailable":
        return (
            f'        <div class="{base} opacity-75" '
            f'aria-disabled="true">\n{inner}\n        </div>'
        )

    return (
        f'        <a class="{base} hover:shadow-md" '
        f'href="{escape(target)}">\n{inner}\n        </a>'
    )


def render_cards(site: Path, reports: list[dict]) -> str:

    if not reports:
        return (
            '        <div class="rounded bg-white shadow border-l-4 '
            'border-l-amber-500 p-4 text-amber-700">'
            "No reports were found and none were expected."
            "</div>"
        )

    return "\n".join(
        render_card(site, report)
        for report in reports
    ).lstrip()


def render_page(
    template: str,
    site: Path,
    reports: list[dict],
    project_name: str,
    commit: str,
    ref: str,
) -> str:

    total_passed = sum(report["passed"] for report in reports)
    total_failed = sum(report["failed"] for report in reports)
    total_cases = sum(report["total"] for report in reports)

    unavailable = sum(
        0 if report["readable"] else 1
        for report in reports
    )

    replacements = {
        "@PROJECT_NAME@": html.escape(project_name),
        "@COMMIT@": html.escape(commit[:12] if commit else "unknown"),
        "@REF@": html.escape(ref or "unknown"),
        "@GENERATED@": datetime.now(timezone.utc).strftime(
            "%Y-%m-%d %H:%M UTC"
        ),
        "@TOTAL_CASES@": str(total_cases),
        "@TOTAL_PASSED@": str(total_passed),
        "@TOTAL_FAILED@": str(total_failed),
        "@TOTAL_UNAVAILABLE@": str(unavailable),
        "@PLATFORM_CARDS@": render_cards(site, reports),
    }

    for token, value in replacements.items():

        # A renamed placeholder must not silently vanish from the page,
        # so an absent token is an error rather than a no-op.
        if token not in template:
            raise InputError(
                f"Template is missing the placeholder '{token}'. "
                f"Check {TEMPLATE_DIRECTORY / 'index.html'}."
            )

        template = template.replace(token, value)

    return template


def main() -> int:

    args = parse_args()

    template_file = TEMPLATE_DIRECTORY / "index.html"
    style_file = TEMPLATE_DIRECTORY / "style.css"

    for path in (template_file, style_file):

        if not path.exists():
            print(f"error: template not found: {path}", file=sys.stderr)
            return 1

    try:
        reports = find_reports(args.site, args.prefix, args.expect)

        if not reports and not args.allow_empty:
            raise InputError(
                f"Nothing was expected and no directories starting with "
                f"'{args.prefix}' were found in {args.site}. Pass "
                f"--expect for each platform that should have run, or "
                f"--allow-empty to write the page anyway."
            )

        page = render_page(
            template_file.read_text(encoding="utf-8"),
            args.site,
            reports,
            args.project_name,
            args.commit,
            args.ref,
        )

    except InputError as error:
        print(f"error: {error}", file=sys.stderr)
        return 1

    (args.site / "index.html").write_text(page, encoding="utf-8")
    shutil.copy2(style_file, args.site / "style.css")

    failed = sum(report["failed"] for report in reports)

    unavailable = sum(
        0 if report["readable"] else 1
        for report in reports
    )

    print(
        f"Wrote {args.site / 'index.html'}: "
        f"{len(reports)} platform(s), "
        f"{failed} failed case(s), "
        f"{unavailable} unavailable report(s)"
    )

    return 2 if (failed or unavailable) else 0


if __name__ == "__main__":
    sys.exit(main())
