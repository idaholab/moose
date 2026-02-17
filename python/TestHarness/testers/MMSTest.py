# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implement MMSTest for testing with the method of manufactured solutions."""

from PythonUnitTest import PythonUnitTest


class MMSTest(PythonUnitTest):
    """
    Use the 'mms' module to test with the method of manufactured solutions.

    MMS tests will not run with:

        - The --valgrind option
        - The --recover option
        - The --restep option
        - The --capture-perf-graph option
        - Any methods other than opt

    """

    @staticmethod
    def validParams():
        """Define valid parameters."""
        params = PythonUnitTest.validParams()

        return params

    def __init__(self, name, params):
        """Initialize."""
        super().__init__(name, params)

        specs = self.specs

        # MMS testing requires pandas and matplotlib
        if specs["required_python_packages"] is None:
            specs["required_python_packages"] = ""
        for package in ["pandas", "matplotlib"]:
            if package not in specs["required_python_packages"]:
                if specs["required_python_packages"]:
                    specs["required_python_packages"] += " "
                specs["required_python_packages"] += package

        # Set max_parallel to whatever min_parallel is. For now because
        # the MMS module doesn't pick up the number of procs from the
        # test harness (and defines it on its own), it can never run
        # greater than whatever min_parallel is.
        if (
            params.isParamSetByUser("max_parallel")
            and specs["max_parallel"] != specs["min_parallel"]
        ):
            raise RuntimeError(
                "Parameter 'max_parallel' must be equal to "
                f"'min_parallel={specs['min_parallel']} for a MMSTest."
            )

        # Require opt mode
        if specs["capabilities"]:
            specs["capabilities"] = "(" + specs["capabilities"] + ") & "
        specs["capabilities"] += "method=opt"
