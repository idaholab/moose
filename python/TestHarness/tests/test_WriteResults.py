#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import shutil, os, subprocess
from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def setUp(self):
        """
        setUp occurs before every test. Clean up previous results file
        """
        self.output_dir = os.path.join(os.getenv('MOOSE_DIR'), 'test', 'WriteResults_OUTPUT')

        try:
            # remove previous results file
            shutil.rmtree(self.output_dir)
        except:
            pass

    def tearDown(self):
        """
        tearDown occurs after every test.
        """
        self.setUp()

    def testWriteOK(self):
        """ Test ability to write separate OK test --sep-files-ok """
        self.runTests('--no-color', '-i', 'always_ok', '--sep-files-ok', '--output-dir', self.output_dir)
        if not os.path.exists(os.path.join(self.output_dir, 'test_harness.always_ok.OK.txt')):
           self.fail('Failed to create sep-files-ok')

        with self.assertRaises(subprocess.CalledProcessError):
            self.runTests('--no-color', '-i', 'diffs', '--sep-files-ok', '--output-dir', self.output_dir)

        if (os.path.exists(os.path.join(self.output_dir, 'test_harness.exodiff.DIFF.txt'))
            or os.path.exists(os.path.join(self.output_dir, 'test_harness.exodiff.OK.txt'))):
           self.fail('Test results which failed were created when asked NOT to do so: --sep-files-ok')

    def testWriteFail(self):
        """ Test ability to write separate Fail test --sep-files-fail """
        with self.assertRaises(subprocess.CalledProcessError):
            self.runTests('--no-color', '-i', 'diffs', '--sep-files-fail', '--output-dir', self.output_dir)

        if not (os.path.exists(os.path.join(self.output_dir, 'test_harness.exodiff.DIFF.txt'))
            and os.path.exists(os.path.join(self.output_dir, 'test_harness.csvdiff.DIFF.txt'))):
           self.fail('Failed to create sep-files-fail')

        self.runTests('--no-color', '-i', 'always_ok', '--sep-files-fail', '--output-dir', self.output_dir)
        if os.path.exists(os.path.join(self.output_dir, 'test_harness.always_ok.OK.txt')):
           self.fail('Test results which passed were created when asked NOT to do so: --sep-files-fail')

    def testWriteAll(self):
        """ Test write all output files --sep-files """
        with self.assertRaises(subprocess.CalledProcessError):
            self.runTests('--no-color', '-i', 'diffs', '--sep-files', '--output-dir', self.output_dir)

        self.runTests('--no-color', '-i', 'always_ok', '--sep-files', '--output-dir', self.output_dir)

        if not (os.path.exists(os.path.join(self.output_dir, 'test_harness.always_ok.OK.txt'))
                and os.path.exists(os.path.join(self.output_dir, 'test_harness.csvdiff.DIFF.txt'))
                and os.path.exists(os.path.join(self.output_dir, 'test_harness.exodiff.DIFF.txt'))):
           self.fail('Failed to create all output files --sep-files')
