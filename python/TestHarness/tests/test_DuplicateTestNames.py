import subprocess
from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testDuplicateTestNames(self):
        """
        Test for duplicate test names
        """

        # Duplicate tests are considered a Fatal Parser Error, hence the 'with assertRaises'
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'duplicate_test_names', '--no-color')

        e = cm.exception

        self.assertRegexpMatches(e.output, r'tests/test_harness.*? \[DUPLICATE TEST\] SKIP')
        self.assertRegexpMatches(e.output, r'tests/test_harness.*?OK')
