#!/usr/bin/env python3
"""
Generates coverage for MOOSE and MOOSE-based applications.

Used primarily in coverage generation on CIVET.
"""

import argparse
import json
import os
import subprocess
import sys
import tempfile
from dataclasses import asdict, dataclass
from importlib.util import find_spec
from pathlib import Path
from typing import Iterable, Optional


class MooseCoverage:
    """Generates coverage for MOOSE and MOOSE-based applications."""

    def __init__(self):
        """Initialize state."""
        self.args = self.parse_args()
        """The parsed arguments."""

    @staticmethod
    def parse_args() -> argparse.Namespace:
        """Parse arguments."""
        parser = argparse.ArgumentParser(
            description="Generates coverage for MOOSE and MOOSE-based applications."
        )

        parent = argparse.ArgumentParser(add_help=False)

        action_parser = parser.add_subparsers(
            dest="action", help="The action to perform."
        )
        action_parser.required = True

        def add_common_args(parser: argparse.ArgumentParser):
            if (MOOSE_JOBS := os.environ.get("MOOSE_JOBS")) is not None:
                default_jobs = int(MOOSE_JOBS)
                default_jobs_help = f"MOOSE_JOBS={default_jobs}"
            else:
                default_jobs = 4
                default_jobs_help = "4"
            parser.add_argument(
                "--jobs",
                "-j",
                type=int,
                default=default_jobs,
                help=f"Number of parallel jobs (default: {default_jobs_help})",
            )
            parser.add_argument(
                "--no-zero", action="store_true", help="Disable zeroing of counters."
            )

        initialize_parser = action_parser.add_parser(
            "initialize", parents=[parent], help="Initializes coverage."
        )
        initialize_parser.add_argument(
            "out_json",
            type=str,
            help="The output path for the initialized .json state file.",
        )
        initialize_parser.add_argument(
            "search_dirs", nargs="*", type=str, help="The directories to search."
        )
        initialize_parser.add_argument(
            "--search-app",
            action="store_true",
            help="Add search directories for an app ('build' and 'src').",
        )
        initialize_parser.add_argument(
            "--allow-contrib",
            action="store_true",
            help="Allow searching for .gcno files in a 'contrib' directory",
        )
        add_common_args(initialize_parser)

        finalize_parser = action_parser.add_parser(
            "finalize", parents=[parent], help="Finalizes coverage."
        )
        add_common_args(finalize_parser)
        finalize_parser.add_argument(
            "in_json", type=str, help="The input path for the initialized .json state."
        )
        finalize_parser.add_argument(
            "out_info", type=str, help="The output path for the finalized .info file."
        )
        finalize_parser.add_argument(
            "include_dirs",
            nargs="*",
            type=str,
            help="The source directories to include in the final output.",
        )

        return parser.parse_args()

    @classmethod
    def warn(cls, content):
        """Print to screen with a warning prefix (yellow)."""
        cls.print(content, prefix_color="yellow", file=sys.stderr)

    @classmethod
    def error(cls, content):
        """Exit with an error with a red prefix."""
        cls.print(content, prefix_color="red", file=sys.stderr)
        raise SystemExit()

    @classmethod
    def add_color(cls, content: str, prefix_color: str):
        """Add color (for on-screen output) to the given content."""
        prefix_color_vals = {"red": 31, "green": 32, "yellow": 33}
        if prefix_color not in prefix_color_vals:
            raise ValueError(f"Unknown prefix color {prefix_color}")
        return "\033[{}m{}\033[0m".format(prefix_color_vals[prefix_color], content)

    @classmethod
    def print(cls, content, prefix_color="green", bold: bool = False, file=sys.stdout):
        """Print to screen with a colored prefix."""
        if bold:
            content = f"\033[1m{content}\033[0m"
        prefix = "[" + cls.add_color("\033[1mcoverage.py\033[0m", prefix_color) + "]"
        print(prefix, content, file=file, flush=True)

    @classmethod
    def run(cls, command):
        """Print a command to screen and run it."""
        cls.print(cls.add_color(" ".join(command), "green"))
        subprocess.run(command, check=True)

    @dataclass
    class GCNODirectory:
        """Data class for a single tracked directory containing .gcno files."""

        dir: str
        """The directory that contains .gcno files."""
        root_dir: str
        """The root directory that was searched."""

    @classmethod
    def find_gcno(
        cls, root_dir: str, ignore_dirs: Optional[Iterable[str]] = None
    ) -> list[GCNODirectory]:
        """Find the directories that contain .gcno files."""
        dirs = []
        for dirpath, _, filenames in os.walk(root_dir):
            if any(v.endswith(".gcno") for v in filenames):
                if ignore_dirs is not None:
                    rel_to_root = os.path.relpath(dirpath, root_dir)
                    if any(rel_to_root.startswith(v) for v in ignore_dirs):
                        continue
                dirs.append(cls.GCNODirectory(dir=dirpath, root_dir=root_dir))
        return dirs

    @staticmethod
    def get_includes(gcno_dirs: Iterable[GCNODirectory]) -> list[str]:
        """Get the include directories given the directories with .gcno files."""
        dirs = set()
        for gcno_dir in gcno_dirs:
            for dir in ["include", "src"]:
                full_dir = os.path.join(gcno_dir.root_dir, dir)
                if full_dir not in dirs and os.path.isdir(full_dir):
                    dirs.add(full_dir)
        return sorted(dirs)

    @staticmethod
    def get_info_source_dirs(info_contents: list[str]) -> list[str]:
        """Get the source directories given a lcov .info file."""
        dirs = set()
        for line in info_contents:
            if line.startswith("SF:"):
                dir = os.path.dirname(line.strip()[3:])
                dirs.add(dir)
        return sorted(dirs)

    @staticmethod
    def pretty_path(path: str) -> str:
        """Get a path for printing, which is relative if within the pwd."""
        if not (relpath := os.path.relpath(path)).startswith(".."):
            return relpath
        return os.path.abspath(path)

    def fastcov_base_command(self) -> list[str]:
        """Get the base command for running fastcov."""
        return [
            "python",
            "-m",
            "fastcov",
            "-j",
            f"{self.args.jobs}",
            "--dump-statistic",
        ]

    def action_initialize(self):
        """Run the initialize action, which initializes coverage."""
        # Load search directories
        search_dirs = self.args.search_dirs
        if self.args.search_app:
            search_dirs.extend(["build", "src"])
        search_dirs = [os.path.abspath(v) for v in search_dirs]
        if not search_dirs:
            self.error("Must provide search directories or --search-app.")

        # Load directories to ignore
        ignore_search_dirs = set()
        if not self.args.allow_contrib:
            ignore_search_dirs.add("contrib/")

        self.print(
            "Initializing coverage for directories "
            + ", ".join([self.pretty_path(v) for v in search_dirs]),
            bold=True,
        )

        # Find directories in search_dirs that have .gcno fiels
        gcno_directories = []
        for search_dir in search_dirs:
            if result := self.find_gcno(search_dir, ignore_search_dirs):
                gcno_directories.extend(result)
            else:
                self.error(
                    f"Failed to find .gcno file(s) in "
                    f"directory {self.pretty_path(search_dir)}"
                )

        # Zero and load initial coverage for each search directory
        with tempfile.TemporaryDirectory() as tempdir:
            individual_infos = []
            for i, gcno_dir in enumerate(gcno_directories):
                self.print(
                    f"Initializing coverage for {self.pretty_path(gcno_dir.dir)}",
                    bold=True,
                )

                if not self.args.no_zero:
                    gcda_files = list(Path(gcno_dir.dir).glob("*.gcda"))
                    if gcda_files:
                        self.print(
                            f"Removing {len(gcda_files)} .gcda files "
                            f"in {self.pretty_path(gcno_dir.dir)}"
                        )
                        [os.remove(f) for f in gcda_files]

                output_path = os.path.join(tempdir, f"initial{i}.info")
                individual_infos.append(output_path)
                cmd = (
                    self.fastcov_base_command()
                    + [
                        "--gcov",
                        "gcov",
                        "--process-gcno",
                        "--lcov",
                        "--output",
                        output_path,
                        "--search-directory",
                        gcno_dir.dir,
                        "--include",
                    ]
                    + self.get_includes(gcno_directories)
                )
                self.run(cmd)

            self.print("Combining initial coverage", bold=True)
            initial_info_path = os.path.join(tempdir, "initial.info")
            cmd = (
                self.fastcov_base_command()
                + ["--add-tracefile"]
                + individual_infos
                + ["--lcov", "--output", initial_info_path]
            )
            self.run(cmd)

            with open(initial_info_path, "r") as f:
                initial_info = f.readlines()

        # Show which directories we have source from
        self.print("Initialized source directories:", bold=True)
        [
            self.print(f"  {self.pretty_path(f)}")
            for f in self.get_info_source_dirs(initial_info)
        ]

        # Store the initial output json
        state = {
            "gcno_directories": [asdict(v) for v in gcno_directories],
            "initial_info": "".join(initial_info),
        }
        out_json_path = os.path.abspath(self.args.out_json)
        self.print(
            f"Writing initial state to {self.pretty_path(out_json_path)}", bold=True
        )
        with open(out_json_path, "w") as f:
            json.dump(state, f)

    def action_finalize(self):
        """Run the finalize action, which finalizes coverage."""
        include_dirs = self.args.include_dirs
        if not include_dirs:
            self.error("Include directories not provided.")
        include_dirs = [os.path.abspath(v) for v in include_dirs]

        in_json_path = self.args.in_json
        self.print(
            f"Reading initial state from {self.pretty_path(in_json_path)}", bold=True
        )
        if not os.path.exists(in_json_path):
            self.error("Initial state not found; must initialize first")

        with open(in_json_path, "r") as f:
            state = json.load(f)
        gcno_directories = [self.GCNODirectory(**v) for v in state["gcno_directories"]]
        initial_info = state["initial_info"]

        search_dirs = sorted(
            set([self.pretty_path(v.root_dir) for v in gcno_directories])
        )
        self.print(
            f"Finalizing coverage in directories {', '.join(search_dirs)}", bold=True
        )

        with tempfile.TemporaryDirectory() as tempdir:
            all_infos = []
            for i, gcno_dir in enumerate(gcno_directories):
                self.print(
                    f"Capturing coverage in {self.pretty_path(gcno_dir.dir)}", bold=True
                )

                if not any(Path(gcno_dir.dir).glob("*.gcda")):
                    self.warn(
                        f"Directory {self.pretty_path(gcno_dir.dir)} does not "
                        "contain coverage (.gcda files); skipping"
                    )
                    continue

                output_path = os.path.join(tempdir, f"covered{i}.info")
                all_infos.append(output_path)

                cmd = (
                    self.fastcov_base_command()
                    + [
                        "--gcov",
                        "gcov",
                        "--lcov",
                        "--output",
                        output_path,
                        "--search-directory",
                        gcno_dir.dir,
                        "--include",
                    ]
                    + include_dirs
                )
                self.run(cmd)

            if not all_infos:
                self.error("No captured coverage was found")

            # Combine captured coverage (without initial)
            self.print("Combining captured coverage", bold=True)
            combined_output_path = os.path.join(tempdir, "covered.info")
            cmd = (
                self.fastcov_base_command()
                + ["--add-tracefile"]
                + all_infos
                + ["--lcov", "--output", combined_output_path]
            )
            self.run(cmd)

            # Read just the directories we captured (not including initial)
            with open(combined_output_path, "r") as f:
                combined_info = f.readlines()

            # Combine final coverage
            self.print("Combining final coverage", bold=True)
            initial_info_path = os.path.join(tempdir, "initial.info")
            with open(initial_info_path, "w") as f:
                f.write(initial_info)
            cmd = (
                self.fastcov_base_command()
                + ["--add-tracefile"]
                + [combined_output_path, initial_info_path]
                + ["--lcov", "--output", self.args.out_info]
                + ["--include"]
                + include_dirs
            )
            self.run(cmd)

        if not self.args.no_zero:
            self.print("Removing .gcda files", bold=True)
            for gcno_dir in gcno_directories:
                gcda_files = list(Path(gcno_dir.dir).glob("*.gcda"))
                if gcda_files:
                    self.print(
                        f"Removing {len(gcda_files)} .gcda files "
                        + f"in {self.pretty_path(gcno_dir.dir)}"
                    )
                    [os.remove(f) for f in gcda_files]

        # Show which directories we have source from
        self.print("Captured source directories:", bold=True)
        [
            self.print(f"  {self.pretty_path(f)}")
            for f in self.get_info_source_dirs(combined_info)
        ]

    def main(self):
        """Run the given action from command line."""
        if find_spec("fastcov") is None:
            self.error("Needed dependency fastcov not found.")

        getattr(self, f"action_{self.args.action}")()


if __name__ == "__main__":
    MooseCoverage().main()
