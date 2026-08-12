#!/usr/bin/env python3
"""Check the generated UTC fixtures against the originals.

generate_utc_fixtures.py writes tests/Shared/Times/UTC from
tests/Shared/Times. Its own --check mode re-runs the same conversion code
and compares, so it only catches a stale file, never a wrong conversion.

This script re-derives every value from scratch instead, so a bug in the
generator shows up as a mismatch rather than being reproduced on both
sides. Run it once after generating and you should not need it again,
since the source fixtures have not moved since June 2021.

Usage:
    python3 tools/fixtures/verify_utc_fixtures.py
"""

from __future__ import annotations

import json
import sys
from datetime import datetime, timezone
from pathlib import Path
from zoneinfo import ZoneInfo

REPO_ROOT = Path(__file__).resolve().parents[2]
SOURCE_DIR = REPO_ROOT / "tests" / "Shared" / "Times"
OUTPUT_DIR = REPO_ROOT / "tests" / "generated" / "Times"

TIME_KEYS = ("fajr", "sunrise", "dhuhr", "asr", "maghrib", "isha")


def utc_name(source: Path) -> Path:
    """Where a source fixture's converted twin lives, e.g. Doha-Qatar_UTC.json."""
    return OUTPUT_DIR / (source.stem + "_UTC.json")


def to_24_hour(clock: str) -> tuple[int, int]:
    """Read a "5:44 AM" style reading as hour and minute on a 24 hour clock."""
    hour_minute, meridiem = clock.split(" ")
    hour, minute = (int(part) for part in hour_minute.split(":"))
    return (hour % 12) + (12 if meridiem.upper() == "PM" else 0), minute


def main() -> int:
    sources = sorted(SOURCE_DIR.glob("*.json"))
    if not sources:
        print("No fixtures found in %s" % SOURCE_DIR, file=sys.stderr)
        return 1

    problems = []
    values = 0

    for source in sources:
        target = utc_name(source)
        if not target.is_file():
            problems.append("%s has no generated counterpart" % source.name)
            continue

        original = json.loads(source.read_text())
        generated = json.loads(target.read_text())
        zone = ZoneInfo(original["params"]["timezone"])

        if "timezone" in generated["params"]:
            problems.append("%s still names a time zone" % target.name)
        if original.get("variance", 0) != generated.get("variance", 0):
            problems.append("%s changed its variance" % target.name)
        if len(original["times"]) != len(generated["times"]):
            problems.append("%s has a different number of entries" % target.name)
            continue

        for before, after in zip(original["times"], generated["times"]):
            if before["date"] != after["date"]:
                problems.append("%s reordered %s" % (target.name, before["date"]))
                continue

            year, month, day = (int(part) for part in before["date"].split("-"))
            for key in TIME_KEYS:
                hour, minute = to_24_hour(before[key])
                local = datetime(year, month, day, hour, minute, tzinfo=zone)
                expected = local.astimezone(timezone.utc).strftime("%Y-%m-%d %H:%M")
                values += 1
                if after[key] != expected:
                    problems.append(
                        "%s %s %s: got %s, worked out %s"
                        % (target.name, before["date"], key, after[key], expected))

    print("%d fixtures, %d time values checked" % (len(sources), values))

    if problems:
        print("\n%d problem(s):" % len(problems), file=sys.stderr)
        for problem in problems[:40]:
            print("  %s" % problem, file=sys.stderr)
        return 1

    print("All conversions independently reproduced")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
