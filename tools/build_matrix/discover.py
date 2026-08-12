from __future__ import annotations

import os
from pathlib import Path


_WINDOWS_EXECUTABLE_SUFFIXES = {
    ".exe",
    ".bat",
    ".cmd",
}

_LIBRARY_SUFFIXES = {
    ".a",
    ".lib",
    ".dll",
    ".so",
    ".dylib",
}


def _is_noise_file(path: Path) -> bool:
    """Files CMake/compilers leave behind that are never real build output."""

    name = path.name

    if name.endswith((".pdb", ".manifest", ".exp", ".ilk")):
        return True

    if name.startswith("cmake") or name.startswith("CMake"):
        return True

    # try_compile()/compiler-id detection binaries and similar scratch files
    # live under CMakeFiles/ and are not build artifacts.
    if "CMakeFiles" in path.parts:
        return True

    return False


def _is_executable(path: Path) -> bool:

    if not path.is_file():
        return False

    if _is_noise_file(path):
        return False

    if path.suffix.lower() in _LIBRARY_SUFFIXES:
        return False

    if os.name == "nt":

        return (
            path.suffix.lower()
            in _WINDOWS_EXECUTABLE_SUFFIXES
        )

    return os.access(
        path,
        os.X_OK,
    )


def _is_library(path: Path) -> bool:

    if not path.is_file():
        return False

    if _is_noise_file(path):
        return False

    return path.suffix.lower() in _LIBRARY_SUFFIXES


def discover_examples(
    build_directory: Path,
    subdirectory: str = "examples",
) -> list[Path]:

    examples_directory = (
        build_directory
        / subdirectory
    )

    if not examples_directory.exists():
        return []

    executables: list[Path] = [
        file.resolve()
        for file in examples_directory.rglob("*")
        if _is_executable(file)
    ]

    executables.sort(
        key=lambda p: (
            len(p.parts),
            str(p),
        )
    )

    return executables


def discover_binaries(
    directory: Path,
) -> list[Path]:
    """Every library or executable produced anywhere under `directory`."""

    if not directory.exists():
        return []

    binaries = [
        file
        for file in directory.rglob("*")
        if _is_executable(file) or _is_library(file)
    ]

    binaries.sort(
        key=lambda p: (
            len(p.parts),
            str(p),
        )
    )

    return binaries


def find_executable(
    directory: Path,
    preferred_name: str | None = None,
) -> Path:
    """
    Find a single produced executable under `directory`, searched
    recursively so it doesn't matter whether the generator placed it
    directly in the build directory or in a per-config subdirectory
    (e.g. multi-config generators on Windows use <build>/<Config>/...).
    """

    candidates = [
        file
        for file in discover_binaries(directory)
        if _is_executable(file)
    ]

    if preferred_name:

        matches = [
            file
            for file in candidates
            if file.stem.lower() == preferred_name.lower()
        ]

        if matches:
            candidates = matches

    if not candidates:

        listing = (
            "\n".join(
                str(path)
                for path in sorted(directory.rglob("*"))
            )
            if directory.exists()
            else "(directory does not exist)"
        )

        raise FileNotFoundError(
            f"Unable to locate an executable under {directory}.\n"
            f"Contents:\n{listing}"
        )

    candidates.sort(
        key=lambda p: (
            len(p.parts),
            str(p),
        )
    )

    return candidates[0]