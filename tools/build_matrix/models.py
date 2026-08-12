from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any


@dataclass(slots=True)
class CommandResult:
    command: list[str]
    cwd: str
    returncode: int
    elapsed: float
    stdout: str
    stderr: str

    # Exit codes that count as success for this specific command. Build
    # system commands like cmake configure, build, and install always use
    # the strict default. Only example programs may override this through
    # the examples section of project.json.
    #
    # This used to be a blanket returncode >= 0 check. That was added so
    # adhan-cli could print usage on exit code 1 without failing a case.
    # The predicate applied to every command though, which silenced cmake
    # errors, doctest failures, and Windows abnormal termination codes
    # like 0xC0000135 for missing DLLs. The per-command expectation below
    # solves the original problem without that collateral damage.
    expected_returncodes: tuple[int, ...] = (0,)

    # Substrings from expect_stdout_contains that were not found in the
    # program output. If this list is non-empty then the command failed
    # even when it exited with an expected code.
    missing_output: list[str] = field(default_factory=list)

    # Set when the process could not start at all due to a missing
    # executable, permission denied, timeout, or similar issue. A launch
    # failure is never treated as success regardless of what
    # expected_returncodes says.
    launch_error: str | None = None

    @property
    def success(self) -> bool:

        if self.launch_error is not None:
            return False

        if self.missing_output:
            return False

        return self.returncode in self.expected_returncodes

    def to_json(self) -> dict[str, Any]:
        return {
            "command": self.command,
            "cwd": self.cwd,
            "returncode": self.returncode,
            "expected_returncodes": list(self.expected_returncodes),
            "missing_output": self.missing_output,
            "launch_error": self.launch_error,
            "elapsed": self.elapsed,
            "success": self.success,
        }


@dataclass(slots=True)
class ExampleResult:
    """
    One invocation of one example program. An example with several
    configured runs produces several of these, distinguished by run_name.
    """

    name: str
    path: str
    success: bool
    returncode: int
    elapsed: float

    run_name: str = ""
    args: list[str] = field(default_factory=list)
    expected_returncodes: list[int] = field(
        default_factory=lambda: [0]
    )
    missing_output: list[str] = field(default_factory=list)
    launch_error: str | None = None

    @property
    def label(self) -> str:
        """Human-readable identifier, e.g. 'adhan-cli / No args'."""

        if not self.run_name:
            return self.name

        return f"{self.name} / {self.run_name}"

    def to_json(self) -> dict[str, Any]:
        return {
            "name": self.name,
            "run_name": self.run_name,
            "label": self.label,
            "path": self.path,
            "args": self.args,
            "success": self.success,
            "returncode": self.returncode,
            "expected_returncodes": self.expected_returncodes,
            "missing_output": self.missing_output,
            "launch_error": self.launch_error,
            "elapsed": self.elapsed,
        }


@dataclass(slots=True)
class CaseResult:
    name: str

    success: bool = False

    configure: CommandResult | None = None
    build: CommandResult | None = None
    tests: CommandResult | None = None
    install: CommandResult | None = None

    # The install check's downstream steps. Recorded so that a failure
    # after a successful install is visible in the report, and so that
    # `steps()` below can state the pass condition explicitly instead of
    # relying on every failure path having returned early.
    consumer_configure: CommandResult | None = None
    consumer_build: CommandResult | None = None
    consumer_run: CommandResult | None = None

    examples: list[ExampleResult] = field(default_factory=list)

    artifacts: list[dict[str, Any]] = field(default_factory=list)

    log_file: str = ""

    # Set when the case was aborted by an unexpected exception (as opposed
    # to a command exiting with an unexpected exit code). A case with
    # `error` set should generally be retried rather than treated as a
    # "real" build failure.
    error: str | None = None

    def steps(self) -> list[CommandResult]:
        """Every command this case actually ran, in execution order."""

        ordered = (
            self.configure,
            self.build,
            self.tests,
            self.install,
            self.consumer_configure,
            self.consumer_build,
            self.consumer_run,
        )

        return [
            step
            for step in ordered
            if step is not None
        ]

    def compute_success(self) -> bool:
        """
        A case passes only if every step it ran succeeded and every example
        it ran exited with an expected code.

        Note the deliberate absence of a special case for "no examples":
        a case that legitimately builds no examples still has to have
        passed configure, build, and (where applicable) tests and install
        to reach this point.
        """

        if self.error is not None:
            return False

        return (
            all(step.success for step in self.steps())
            and all(example.success for example in self.examples)
        )

    def to_json(self) -> dict[str, Any]:

        def encode(step: CommandResult | None) -> dict[str, Any] | None:
            return None if step is None else step.to_json()

        return {
            "name": self.name,
            "success": self.success,
            "log_file": self.log_file,
            "error": self.error,
            "configure": encode(self.configure),
            "build": encode(self.build),
            "tests": encode(self.tests),
            "install": encode(self.install),
            "consumer_configure": encode(self.consumer_configure),
            "consumer_build": encode(self.consumer_build),
            "consumer_run": encode(self.consumer_run),
            "examples": [
                example.to_json()
                for example in self.examples
            ],
            "artifacts": self.artifacts,
        }
