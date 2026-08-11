from __future__ import annotations

import json
from dataclasses import dataclass, field
from enum import Enum
from pathlib import Path


# These two enums describe the fixed axes of the matrix: every project has
# a shared/static choice and a set of CMake build types. They live here,
# in the config layer, so that project.json can be validated against them
# without matrix.py and project_config.py importing each other.
class LibraryType(Enum):
    SHARED = "shared"
    STATIC = "static"


class BuildType(Enum):
    DEBUG = "Debug"
    RELEASE = "Release"
    RELWITHDEBINFO = "RelWithDebInfo"
    MINSIZEREL = "MinSizeRel"


# The tool ships with a default project.json (next to this file) so it works
# out of the box for the project it was written for. To reuse these tools in
# a different CMake project, either edit that file or point --project-config
# at a different one; nothing else in this package needs to change.
DEFAULT_PROJECT_CONFIG_PATH = (
    Path(__file__).resolve().parent
    / "project.json"
)


@dataclass(slots=True)
class OptionSpec:
    """A single boolean CMake option that becomes an axis of the matrix."""

    id: str
    cmake_var: str


@dataclass(slots=True)
class ExampleRun:
    """
    One invocation of one example program.

    `name` labels the run in logs and the report ("No args", "Polar
    location"). It is optional; an unnamed run is labelled by position.

    `expected_exit_code` is normally a single integer. A list is accepted
    for the rare program whose exit code legitimately varies.

    `expect_stdout_contains` turns "it exited 0" into "it did the thing":
    each substring must appear in the program's stdout. Optional, and
    worth using at least once per example, because an exit code alone
    cannot distinguish a working program from one that printed a usage
    screen and gave up.

    `timeout_seconds` overrides the project-level default for this run.
    """

    name: str = ""
    args: list[str] = field(default_factory=list)
    expected_exit_codes: tuple[int, ...] = (0,)
    expect_stdout_contains: list[str] = field(default_factory=list)
    timeout_seconds: float | None = None


@dataclass(slots=True)
class ExampleSpec:
    """
    How to exercise one built example.

    `name` matches the executable's stem, so it is spelled without any
    platform suffix ("adhan-cli", not "adhan-cli.exe"). Matching is
    case-insensitive.

    An example with no configured runs gets a single default run: no
    arguments, must exit 0.
    """

    name: str
    runs: list[ExampleRun] = field(
        default_factory=lambda: [ExampleRun(name="default")]
    )


@dataclass(slots=True)
class ProjectConfig:

    package_name: str = "package"
    link_target: str = "package::package"

    shared_libs_var: str = "BUILD_SHARED_LIBS"
    build_type_var: str = "CMAKE_BUILD_TYPE"
    cmake_minimum_version: str = "3.21"

    # Which CMake build types the sweep covers. Defaults to all of them;
    # narrow it in project.json to trade coverage for wall-clock time.
    # Debug and Release are the two that differ semantically (NDEBUG on
    # or off); RelWithDebInfo and MinSizeRel only vary the optimisation
    # level, so they rarely surface a bug the other two miss.
    build_types: list[BuildType] = field(
        default_factory=lambda: list(BuildType)
    )

    # Whether the sweep covers installing, not installing, or both.
    # Defaults to both. Pinning to [True] halves the case count at
    # almost no cost: an install-off case builds a strict subset of what
    # an install-on case builds before it installs.
    install_choices: list[bool] = field(
        default_factory=lambda: [False, True]
    )

    test_target: str = "test"
    examples_subdirectory: str = "examples"

    # id of the OptionSpec (below) that gates whether the test target is
    # run / examples are expected to exist. Set to None if not applicable.
    tests_option: str | None = None
    examples_option: str | None = None

    options: list[OptionSpec] = field(
        default_factory=list
    )

    # Per-example invocation settings. Any discovered executable with no
    # entry here is run once with no arguments and must exit 0.
    examples: list[ExampleSpec] = field(
        default_factory=list
    )

    # Default wall-clock limit for a single example run, in seconds.
    # None means no limit (the historical behaviour).
    example_timeout_seconds: float | None = None

    def option_ids(self) -> list[str]:
        return [option.id for option in self.options]

    def example_for(self, stem: str) -> ExampleSpec:
        """
        Look up the spec for a discovered executable by its stem, falling
        back to "no arguments, must exit 0" for anything unconfigured.
        """

        lowered = stem.lower()

        for example in self.examples:

            if example.name.lower() == lowered:
                return example

        return ExampleSpec(name=stem)

    def builds_examples(self, options: dict[str, bool]) -> bool:
        """
        Whether a case with these option values is expected to produce
        example executables. Returns True when no option gates them.
        """

        if self.examples_option is None:
            return True

        return options.get(self.examples_option, False)


def _parse_build_types(value: object) -> list[BuildType]:
    """
    Resolve the 'build_types' field to enum members.

    Anything unrecognised is an error rather than a silent skip: a typo
    like "Realease" would otherwise quietly shrink the sweep, and a run
    that covered less than you thought is indistinguishable from one
    that covered everything and passed.
    """

    valid = ", ".join(build_type.value for build_type in BuildType)

    if not isinstance(value, list) or not value:
        raise ValueError(
            f"'build_types' must be a non-empty list of CMake build type "
            f"names. Omit the field entirely to sweep all of: {valid}."
        )

    by_name = {
        build_type.value.lower(): build_type
        for build_type in BuildType
    }

    resolved: list[BuildType] = []

    for entry in value:

        if not isinstance(entry, str):
            raise ValueError(
                f"'build_types': expected a string, got "
                f"{type(entry).__name__}. Valid values are: {valid}."
            )

        build_type = by_name.get(entry.strip().lower())

        if build_type is None:
            raise ValueError(
                f"'build_types': unknown build type '{entry}'. "
                f"Valid values are: {valid}."
            )

        if build_type in resolved:
            raise ValueError(
                f"'build_types': duplicate entry '{entry}'."
            )

        resolved.append(build_type)

    return resolved


def _parse_install(value: object) -> list[bool]:
    """
    Resolve the 'install' field to the set of values that axis takes.

    Accepts a single boolean to pin the axis, or a two-element list to
    sweep both. Anything else is an error, for the same reason as
    'build_types': a config that quietly covers less than intended is
    worse than one that refuses to load.
    """

    if isinstance(value, bool):
        return [value]

    if isinstance(value, list) and value:

        resolved: list[bool] = []

        for entry in value:

            if not isinstance(entry, bool):
                raise ValueError(
                    f"'install': expected true or false, got "
                    f"{type(entry).__name__}."
                )

            if entry in resolved:
                raise ValueError(
                    f"'install': duplicate entry {str(entry).lower()}."
                )

            resolved.append(entry)

        return resolved

    raise ValueError(
        "'install' must be true, false, or a non-empty list of booleans. "
        "Omit the field entirely to sweep both."
    )


def _parse_exit_codes(value: object, where: str) -> tuple[int, ...]:

    # A single integer is the normal case; a list is allowed for programs
    # whose exit code legitimately varies.
    if isinstance(value, bool):
        raise ValueError(
            f"{where}: 'expected_exit_code' must be an integer, not a boolean."
        )

    if isinstance(value, int):
        return (value,)

    if isinstance(value, list) and value:
        return tuple(int(code) for code in value)

    raise ValueError(
        f"{where}: 'expected_exit_code' must be an integer, or a non-empty "
        f"list of integers."
    )


def _parse_run(entry: object, example_name: str, position: int) -> ExampleRun:

    where = f"example '{example_name}', run {position}"

    if not isinstance(entry, dict):
        raise ValueError(
            f"{where}: expected an object, got {type(entry).__name__}."
        )

    unknown = set(entry) - {
        "name",
        "args",
        "expected_exit_code",
        "expect_stdout_contains",
        "timeout_seconds",
    }

    if unknown:
        raise ValueError(
            f"{where}: unknown field(s) {sorted(unknown)}. "
            f"Check for a typo - a misspelled field would otherwise be "
            f"silently ignored."
        )

    timeout = entry.get("timeout_seconds")

    return ExampleRun(
        name=str(entry.get("name", "")),
        args=[str(arg) for arg in entry.get("args", [])],
        expected_exit_codes=_parse_exit_codes(
            entry.get("expected_exit_code", 0),
            where,
        ),
        expect_stdout_contains=[
            str(text) for text in entry.get("expect_stdout_contains", [])
        ],
        timeout_seconds=None if timeout is None else float(timeout),
    )


def _parse_example(entry: object) -> ExampleSpec:

    # A bare string is shorthand for "run it once with no arguments,
    # expect 0".
    if isinstance(entry, str):
        return ExampleSpec(name=entry)

    if not isinstance(entry, dict):
        raise ValueError(
            f"Invalid entry in 'examples': expected a string or an object, "
            f"got {type(entry).__name__}."
        )

    if "name" not in entry:
        raise ValueError(
            "Every entry in 'examples' needs a 'name' field naming the "
            "executable (without any platform suffix)."
        )

    name = str(entry["name"])

    unknown = set(entry) - {"name", "runs"}

    if unknown:
        raise ValueError(
            f"example '{name}': unknown field(s) {sorted(unknown)}. "
            f"Per-invocation settings belong inside 'runs'."
        )

    if "runs" not in entry:
        return ExampleSpec(name=name)

    raw_runs = entry["runs"]

    if not isinstance(raw_runs, list) or not raw_runs:
        raise ValueError(
            f"example '{name}': 'runs' must be a non-empty list. Omit the "
            f"field entirely to get the default single no-argument run."
        )

    runs = [
        _parse_run(raw, name, position)
        for position, raw in enumerate(raw_runs, start=1)
    ]

    # Auto-label unnamed runs so every result is identifiable in the
    # report, and reject duplicates so two runs never collide there.
    seen: set[str] = set()

    for position, run in enumerate(runs, start=1):

        if not run.name:
            run.name = f"run {position}"

        if run.name in seen:
            raise ValueError(
                f"example '{name}': duplicate run name '{run.name}'."
            )

        seen.add(run.name)

    return ExampleSpec(name=name, runs=runs)


def load_project_config(
    path: Path | None,
) -> ProjectConfig:
    """
    Load project.json (or an explicitly provided config file). Missing
    optional fields fall back to sensible defaults so a minimal config
    file is enough to onboard a new project.
    """

    config_path = path or DEFAULT_PROJECT_CONFIG_PATH

    if not config_path.exists():

        if path is not None:
            raise FileNotFoundError(
                f"Project config not found: {config_path}"
            )

        return ProjectConfig()

    data = json.loads(
        config_path.read_text(encoding="utf-8")
    )

    options = [
        OptionSpec(
            id=option["id"],
            cmake_var=option["cmake_var"],
        )
        for option in data.get("options", [])
    ]

    examples = [
        _parse_example(entry)
        for entry in data.get("examples", [])
    ]

    defaults = ProjectConfig()

    # A missing key means "sweep everything"; a present key is validated
    # strictly. Absence and a typo must not lead to the same place.
    build_types = (
        defaults.build_types
        if "build_types" not in data
        else _parse_build_types(data["build_types"])
    )

    install_choices = (
        defaults.install_choices
        if "install" not in data
        else _parse_install(data["install"])
    )

    return ProjectConfig(
        package_name=data.get("package_name", defaults.package_name),
        link_target=data.get("link_target", defaults.link_target),
        shared_libs_var=data.get("shared_libs_var", defaults.shared_libs_var),
        build_type_var=data.get("build_type_var", defaults.build_type_var),
        cmake_minimum_version=data.get(
            "cmake_minimum_version", defaults.cmake_minimum_version
        ),
        build_types=build_types,
        install_choices=install_choices,
        test_target=data.get("test_target", defaults.test_target),
        examples_subdirectory=data.get(
            "examples_subdirectory", defaults.examples_subdirectory
        ),
        tests_option=data.get("tests_option", defaults.tests_option),
        examples_option=data.get("examples_option", defaults.examples_option),
        options=options,
        examples=examples,
        example_timeout_seconds=data.get(
            "example_timeout_seconds", defaults.example_timeout_seconds
        ),
    )