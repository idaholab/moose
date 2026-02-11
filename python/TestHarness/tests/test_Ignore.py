# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test the TestHarness --ignore option."""

from unittest import main

from TestHarnessTestCase import TestHarnessTestCase


class TestHarnessTester(TestHarnessTestCase):
    """Test the TestHarness --ignore option."""

    def testIgnoreSkip(self):
        """Test `--ignore skip` running tests normally skipped."""
        # Run a skipped test
        output = self.runTests("-i", "ignore_skipped", "--ignore", "skip").output
        self.assertRegex(output, r"test_harness\.ignore_skipped.*?OK")

    def testIgnoreHeavy(self):
        """Test `--ignore heavy` running tests normally skipped if heavy."""
        # Run a skipped heavy test
        output = self.runTests("-i", "ignore_heavy", "--ignore", "heavy").output
        self.assertRegex(output, r"test_harness\.ignore_heavy.*?OK")

    def testIgnorePreReq(self):
        """Test `--ignore prereq` running tests normally skipped with prereqs."""
        # Run a skipped prereq test
        output = self.runTests(
            "--no-color", "-i", "ignore_prereq", "--ignore", "prereq"
        ).output
        self.assertRegex(
            output, r"test_harness\.always_skipped.*? \[ALWAYS SKIPPED\] SKIP"
        )
        self.assertRegex(output, r"test_harness\.ignore_skipped_dependency.*?OK")

        # Check that a dependency test runs when its prereq test is skipped
        output = self.runTests(
            "--no-color", "-i", "ignore_prereq", "--ignore", "skip"
        ).output
        self.assertRegex(output, r"test_harness\.always_skipped.*?OK")
        self.assertRegex(output, r"test_harness\.ignore_skipped_dependency.*?OK")

    def testIgnoreMultiple(self):
        """Test `--ignore [list]` running tests that would be skipped."""
        # Run a multiple caveat skipped test by manually supplying each caveat
        output = self.runTests("-i", "ignore_multiple", "--ignore", "skip heavy").output
        self.assertRegex(output, r"test_harness\.ignore_multiple.*?OK")

    def testIgnoreAll(self):
        """Test `--ignore` running anything that would normally be skipped."""
        # Run a multiple caveat skipped test using built in default 'all'
        output = self.runTests("-i", "ignore_multiple", "--ignore").output
        self.assertRegex(output, r"test_harness\.ignore_multiple.*?OK")

    def testIgnoreMissingOne(self):
        """Test `--ignore [list]` with one caveat missing will still skip."""
        # Skip a multiple caveat test by not supplying enough caveats to ignore
        output = self.runTests(
            "--no-color", "-i", "ignore_multiple", "--ignore", "skip"
        ).output
        self.assertRegex(output, r"test_harness\.ignore_multiple.*? \[HEAVY\] SKIP")

    def testIgnoreMultiplePreReq(self):
        """Test `--ignore [assorted]` on tests with multiple prereqs."""
        # Run a multiple caveat prereq test using built in default 'all'
        output = self.runTests("-i", "ignore_multiple_prereq", "--ignore").output
        self.assertRegex(output, r"test_harness\.always_skipped.*?OK")
        self.assertRegex(output, r"test_harness\.ignore_multi_prereq_dependency.*?OK")

        # Run a multiple caveat prereq test by manually supplying each caveat
        output = self.runTests(
            "-i", "ignore_multiple_prereq", "--ignore", "prereq skip heavy"
        ).output
        self.assertRegex(output, r"test_harness\.always_skipped.*?OK")
        self.assertRegex(output, r"test_harness\.ignore_multi_prereq_dependency.*?OK")

        # Check that a multiple caveat dependency test runs when its prereq test
        # is skipped; this test may seem redundant, but `prereq` is handled
        # differently than the other caveats
        output = self.runTests(
            "--no-color",
            "-i",
            "ignore_multiple_prereq",
            "--ignore",
            "prereq heavy compiler platform",
        ).output
        self.assertRegex(
            output, r"test_harness\.always_skipped.*? \[ALWAYS SKIPPED\] SKIP"
        )
        self.assertRegex(output, r"test_harness\.ignore_multi_prereq_dependency.*?OK")

        # Check that by supplying a very specific set of ignored paramaters, we
        # can properly trigger a skipped dependency scenario
        output = self.runTests(
            "--no-color",
            "-i",
            "ignore_multiple_prereq",
            "--ignore",
            "heavy compiler platform",
        ).output
        self.assertRegex(
            output, r"test_harness\.always_skipped.*? \[ALWAYS SKIPPED\] SKIP"
        )
        self.assertRegex(
            output,
            r"test_harness\.ignore_multi_prereq_dependency.*? \[SKIPPED DEPENDENCY\] SKIP",
        )


if __name__ == "__main__":
    main()
