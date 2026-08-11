from __future__ import annotations

from dataclasses import dataclass, field
from itertools import product

# LibraryType and BuildType now live in project_config so that project.json
# can be validated against them. They are re-exported here because runner.py
# and any external caller still import LibraryType from this module.
from project_config import BuildType, LibraryType, ProjectConfig

__all__ = [
    "BuildType",
    "LibraryType",
    "BuildCase",
    "generate_build_matrix",
]


@dataclass(slots=True)
class BuildCase:
    name: str

    library: LibraryType
    build_type: BuildType

    # One entry per OptionSpec in the project config, e.g.
    # {"build_tests": True, "build_examples": False, ...}
    options: dict[str, bool]

    run_tests: bool
    install: bool

    metadata: dict[str, object] = field(
        default_factory=dict
    )


def _bool_name(
    value: bool,
) -> str:

    return "on" if value else "off"


def _case_name(
    *,
    library: LibraryType,
    build_type: BuildType,
    options: dict[str, bool],
    option_order: list[str],
    run_tests: bool,
    tests_option: str | None,
    install: bool,
) -> str:

    parts = [
        library.value,
        build_type.name.lower(),
    ]

    for option_id in option_order:
        parts.append(
            f"{option_id}-{_bool_name(options[option_id])}"
        )

    if tests_option is not None and options.get(tests_option, False):
        parts.append(
            f"run-tests-{_bool_name(run_tests)}"
        )

    parts.append(
        f"install-{_bool_name(install)}"
    )

    return "-".join(parts)


def generate_build_matrix(
    project: ProjectConfig,
) -> list[BuildCase]:

    option_order = project.option_ids()

    # One (False, True) pair per configured option, in order.
    option_choices = [
        (False, True)
        for _ in option_order
    ]

    matrix: list[BuildCase] = []

    for (
        library,
        build_type,
        install,
        *option_values,
    ) in product(
        LibraryType,
        project.build_types,
        project.install_choices,
        *option_choices,
    ):

        options = dict(
            zip(option_order, option_values)
        )

        run_tests = (
            options.get(project.tests_option, False)
            if project.tests_option is not None
            else False
        )

        case = BuildCase(
            name=_case_name(
                library=library,
                build_type=build_type,
                options=options,
                option_order=option_order,
                run_tests=run_tests,
                tests_option=project.tests_option,
                install=install,
            ),
            library=library,
            build_type=build_type,
            options=options,
            run_tests=run_tests,
            install=install,
        )

        case.metadata = {
            "library": library.value,
            "build_type": build_type.value,
            "run_tests": run_tests,
            "install": install,
            **options,
        }

        matrix.append(
            case
        )

    matrix.sort(
        key=lambda case: case.name
    )

    return matrix