# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test validation case for TestHarness.resultsstore."""

from TestHarness.validation import ValidationCase


class TestCase(ValidationCase):
    """Validation case for testing TestHarness.resultsstore."""

    def initialize(self):
        """Initialize; loads the value of interest."""
        csv_file = self.getTesterOutputs(extension="csv")[0]
        import sys

        print(sys.path)

        from pandas import read_csv

        df = read_csv(csv_file)
        self.value = float(df["value"].iloc[-1])

    def testValidation(self):
        """Add a simple validation test case that produces validation data."""
        self.addScalarData(
            "number",
            self.value,
            "Number",
            "coolunits",
            bounds=(self.value - 0.01, self.value + 0.01),
        )

        self.addScalarData(
            "unbounded_number",
            self.value,
            "Number",
            "coolunits",
        )
