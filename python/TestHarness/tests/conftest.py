# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Configures pytest for the TestHarness module."""

import pytest

pytest_plugins = (
    # Configures pytest for finding and optionally
    # skipping tests that depend on MOOSE.
    "moosepytest.plugins.mooseexe",
)


def pytest_addoption(parser: pytest.Parser):
    """Add the --no-live-db option to the Parser."""
    parser.addoption(
        "--no-live-db",
        action="store_true",
        default=False,
        help="Skip tests that use the live database",
    )


def pytest_collection_modifyitems(config: pytest.Config, items: list[pytest.Item]):
    """Modify items to skip live database tests if --no-live-db is set."""
    # Skip 'moose' tests if --no-moose is set
    if config.getoption("--no-live-db"):
        marker = pytest.mark.skip(reason="--no-live-db")
        for item in items:
            if "live_db" in item.keywords:
                item.add_marker(marker)


def pytest_configure(config: pytest.Config):
    """Notify when tests are ran with --no-live-db."""
    if config.getoption("--no-live-db"):
        print("INFO: Not running tests that utilize a results database")
