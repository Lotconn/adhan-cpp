from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path


@dataclass(slots=True)
class BuildMatrixConfig:
    source: Path
    output: Path

    # Per-build parallelism, i.e. `cmake --build <dir> --parallel <jobs>`.
    jobs: int | None = None

    # How many cases to build concurrently.
    workers: int = 1

    verbose: bool = False

    # CMake generator to use, e.g. "Ninja". None means "let CMake decide".
    generator: str | None = None

    @property
    def logs(self) -> Path:
        return self.output / "logs"

    @property
    def cases(self) -> Path:
        return self.output / "cases"

    @property
    def builds(self) -> Path:
        return self.output / "builds"

    @property
    def install(self) -> Path:
        return self.output / "install"

    @property
    def consumer(self) -> Path:
        return self.output / "consumer"

    @property
    def html(self) -> Path:
        return self.output / "html"

    @property
    def summary_json(self) -> Path:
        return self.output / "matrix.json"

    @property
    def metadata_json(self) -> Path:
        return self.output / "metadata.json"

    @property
    def template_directory(self) -> Path:
        return (
            Path(__file__).resolve().parent
            / "templates"
        )

    @property
    def html_directory(self) -> Path:
        return (
            Path(__file__).resolve().parent
            / "templates/html"
        )

    @property
    def report_directory(self) -> Path:
        return self.output

    def build_directory(
        self,
        case_name: str,
    ) -> Path:
        return (
            self.builds
            / case_name
        )

    def install_directory(
        self,
        case_name: str,
    ) -> Path:
        return (
            self.install
            / case_name
        )

    def consumer_directory(
        self,
        case_name: str,
    ) -> Path:
        return (
            self.consumer
            / case_name
        )

    def case_json(
        self,
        case_name: str,
    ) -> Path:
        return (
            self.cases
            / f"{case_name}.json"
        )

    def log_file(
        self,
        case_name: str,
    ) -> Path:
        return (
            self.logs
            / f"{case_name}.log"
        )

    def ensure_directories(
        self,
    ) -> None:

        directories = (
            self.output,
            self.logs,
            self.cases,
            self.builds,
            self.install,
            self.consumer,
            self.html,
        )

        for directory in directories:

            directory.mkdir(
                parents=True,
                exist_ok=True,
            )
