#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testIgnoreSkip(self):
        """
        Test that `--ignore skip` runs tests normally skipped
        """
        # Run a skipped test
        output = self.runTests('-i', 'ignore_skipped', '--ignore', 'skip')
        self.assertRegex(output.decode('utf-8'), 'test_harness\.ignore_skipped.*?OK')

    def testIgnoreHeavy(self):
        """
        Test that `--ignore heavy` runs tests normally skipped if heavy
        """
        # Run a skipped heavy test
        output = self.runTests('-i', 'ignore_heavy', '--ignore', 'heavy')
        self.assertRegex(output.decode('utf-8'), 'test_harness\.ignore_heavy.*?OK')

    def testIgnoreCompiler(self):
        """
        Test that `--ignore compiler` runs tests normally skipped if compiler
        is not available
        """
        # Run a skipped compiler test
        output = self.runTests('-i', 'ignore_compiler', '--ignore', 'compiler')
        self.assertRegex(output.decode('utf-8'), 'test_harness\.ignore_compiler.*?OK')

    def testIgnorePlatform(self):
        """
        Test that `--ignore platform` runs tests normally skipped if platform
        is not available
        """
        # Run a skipped platform test
        output = self.runTests('-i', 'ignore_platform', '--ignore', 'platform')
        self.assertRegex(output.decode('utf-8'), 'test_harness\.ignore_platform.*?OK')

    def testIgnorePreReq(self):
        """
        Tests that `--ignore prereq` runs tests normally skipped if prereqs
        are not satisfied
        """
        # Run a skipped prereq test
        output = self.runTests('--no-color', '-i', 'ignore_prereq', '--ignore', 'prereq')
        self.assertRegex(output.decode('utf-8'), 'test_harness\.always_skipped.*? \[ALWAYS SKIPPED\] SKIP')
        self.assertRegex(output.decode('utf-8'), 'test_harness\.ignore_skipped_dependency.*?OK')

        # Check that a dependency test runs when its prereq test is skipped
        output = self.runTests('--no-color', '-i', 'ignore_prereq', '--ignore', 'skip')
        self.assertRegex(output.decode('utf-8'), 'test_harness\.always_skipped.*?OK')
        self.assertRegex(output.decode('utf-8'), 'test_harness\.ignore_skipped_dependency.*?OK')

    def testIgnoreMultiple(self):
        """
        Test that `--ignore [list]` runs tests with multiple caveats
        preventing the test from running
        """
        # Run a multiple caveat skipped test by manually supplying each caveat
        output = self.runTests('-i', 'ignore_multiple', '--ignore', 'skip heavy compiler platform')
        self.assertRegex(output.decode('utf-8'), 'test_harness\.ignore_multiple.*?OK')

    def testIgnoreAll(self):
        """
        Test that the blanket option `--ignore` will run anything that would
        normally be skipped
        """
        # Run a multiple caveat skipped test using built in default 'all'
        output = self.runTests('-i', 'ignore_multiple', '--ignore')
        self.assertRegex(output.decode('utf-8'), 'test_harness\.ignore_multiple.*?OK')

    def testIgnoreMissingOne(self):
        """
        Test that `--ignore [list]` (but missing one) will still have that
        test skipped (platform not ignored)
        """
        # Skip a multiple caveat test by not supplying enough caveats to ignore
        output = self.runTests('--no-color', '-i', 'ignore_multiple', '--ignore', 'skip heavy compiler')
        self.assertRegex(output.decode('utf-8'), 'test_harness\.ignore_multiple.*? \[PLATFORM!=NON_EXISTENT\] SKIP')

    def testIgnoreMultiplePreReq(self):
        """
        Test that `--ignore [assorted]` on a multi-required caveat test
        operates the way it should
        """
        # Run a multiple caveat prereq test using built in default 'all'
        output = self.runTests('-i', 'ignore_multiple_prereq', '--ignore')
        self.assertRegex(output.decode('utf-8'), 'test_harness\.always_skipped.*?OK')
        self.assertRegex(output.decode('utf-8'), 'test_harness\.ignore_multi_prereq_dependency.*?OK')

        # Run a multiple caveat prereq test by manually supplying each caveat
        output = self.runTests('-i', 'ignore_multiple_prereq', '--ignore', 'prereq skip heavy compiler platform')
        self.assertRegex(output.decode('utf-8'), 'test_harness\.always_skipped.*?OK')
        self.assertRegex(output.decode('utf-8'), 'test_harness\.ignore_multi_prereq_dependency.*?OK')

        # Skip a multiple caveat prereq test by not supplying enough caveats to ignore
        output = self.runTests('--no-color', '-i', 'ignore_multiple_prereq', '--ignore', 'prereq skip heavy compiler')
        self.assertRegex(output.decode('utf-8'), 'test_harness\.always_skipped.*?OK')
        self.assertRegex(output.decode('utf-8'), 'test_harness\.ignore_multi_prereq_dependency.*? \[PLATFORM!=NON_EXISTENT\] SKIP')

        # Check that a multiple caveat dependency test runs when its prereq test is skipped
        # This test may seem redundant, but `prereq` is handled differently than the other caveats
        output = self.runTests('--no-color', '-i', 'ignore_multiple_prereq', '--ignore', 'prereq heavy compiler platform')
        self.assertRegex(output.decode('utf-8'), 'test_harness\.always_skipped.*? \[ALWAYS SKIPPED\] SKIP')
        self.assertRegex(output.decode('utf-8'), 'test_harness\.ignore_multi_prereq_dependency.*?OK')

        # Check that by supplying a very specific set of ignored paramaters, we
        # can properly trigger a skipped dependency scenario
        output = self.runTests('--no-color', '-i', 'ignore_multiple_prereq', '--ignore', 'heavy compiler platform')
        self.assertRegex(output.decode('utf-8'), 'test_harness\.always_skipped.*? \[ALWAYS SKIPPED\] SKIP')
        self.assertRegex(output.decode('utf-8'), 'test_harness\.ignore_multi_prereq_dependency.*? \[SKIPPED DEPENDENCY\] SKIP')
