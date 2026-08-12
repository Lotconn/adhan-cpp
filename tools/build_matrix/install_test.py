from __future__ import annotations

import shutil
from pathlib import Path


def create_consumer_project(
    template_directory: Path,
    project_directory: Path,
    install_prefix: Path,
    package_name: str,
    link_target: str,
    cmake_minimum_version: str,
    build_type: str,
) -> None:
    """
    Create a temporary CMake project used to verify that the installed
    package can be located with find_package() and linked against by an
    external project.
    """

    if project_directory.exists():
        shutil.rmtree(project_directory)

    shutil.copytree(
        template_directory,
        project_directory,
    )

    cmake_file = (
        project_directory
        / "CMakeLists.txt"
    )

    contents = cmake_file.read_text(
        encoding="utf-8",
    )

    # Forward slashes are always valid in CMake path arguments, on every
    # platform, so this sidesteps any Windows backslash-escaping issues.
    prefix = install_prefix.resolve().as_posix()

    replacements = {
        "@CMAKE_MINIMUM_VERSION@": cmake_minimum_version,
        "@PACKAGE_NAME@": package_name,
        "@LINK_TARGET@": link_target,
        "@INSTALL_PREFIX@": prefix,
        "@CASE_BUILD_TYPE@": build_type,
    }

    for token, value in replacements.items():

        if token not in contents:
            raise RuntimeError(
                f"Consumer template is missing the expected placeholder "
                f"'{token}'. Check tools/build_matrix/templates/CMakeLists.txt."
            )

        contents = contents.replace(
            token,
            value,
        )

    cmake_file.write_text(
        contents,
        encoding="utf-8",
    )