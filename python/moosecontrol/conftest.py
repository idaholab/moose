#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import pytest

def pytest_addoption(parser):
    """
    Adds custom options to pytest.
    """
    parser.addoption(
        "--no-moose",
        action="store_true",
        default=False,
        help="Skip tests that require a moose executable"
    )

def pytest_collection_modifyitems(config, items):
    """
    Adds custom skips to pytest.
    """
    # Skip 'moose' tests if --no-moose is set
    if config.getoption("--no-moose"):
        marker = pytest.mark.skip(reason="skipping moose test due to --no-moose")
        for item in items:
            if "moose" in item.keywords:
                item.add_marker(marker)
