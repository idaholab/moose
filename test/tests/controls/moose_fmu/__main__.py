# This test is generated according to examples in FMU4FOAM.
# FMU4FOAM is an open-source tool developed by the German Aerospace Center (DLR) for
# coupling OpenFOAM with other simulation environments via the Functional Mock-up Interface (FMI).
# For more information, please refer to the FMU4FOAM GitHub repository:
# https://github.com/DLR-RY/FMU4FOAM
#
# We acknowledge the developers of FMU4FOAM for providing valuable examples and resources
# that have significantly aided in the development of this test.

import argparse

from FMU4MOOSE._version import __version__
from FMU4MOOSE import builder


def cli_main():
    parser = argparse.ArgumentParser(prog="fmu4MOOSE")

    parser.add_argument("-V", "--version", action="version", version=__version__)

    def default_execution(**kwargs):
        print("A subcommand must be provided.\n")
        parser.print_help()

    parser.set_defaults(execute=default_execution)

    subparsers = parser.add_subparsers(
        title="Subcommands",
        description="Call `fmu4MOOSE _command_ -h` to get more help.",
    )

    build_parser = subparsers.add_parser(
        "build",
        description="Build an FMU from a Python script.",
        help="Build an FMU from a Python script.",
    )
    builder.create_command_parser(build_parser)

    options = vars(parser.parse_args())
    execute = options.pop("execute")
    execute(**options)


if __name__ == "__main__":
    cli_main()
