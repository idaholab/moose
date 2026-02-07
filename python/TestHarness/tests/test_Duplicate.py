# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test the TestHarnes finding duplicate output files."""

from TestHarnessTestCase import TestHarnessTestCase


class TestDuplicate(TestHarnessTestCase):
    """Test the TestHarnes finding duplicate output files."""

    def testDuplicateOutputs(self):
        """Test for duplicate output files in the same directory."""
        def run_test(spec, test_names):
            test_names = sorted(test_names)
            result = self.runTests('-i', spec, exit_code=132)
            out = result.output
            harness = result.harness
            assert harness is not None

            for name in test_names:
                job = self.getJobWithName(harness, name)
                files = job.getOutputFiles(job.options)
                self.assertIn(f'Tests: {", ".join(test_names)}', out)
                self.assertIn(f'File(s): {", ".join(files)}', out)

        run_test('duplicate_outputs', ['a', 'b', 'c', 'd'])
        run_test('duplicate_outputs_analyzejacobian', ['a', 'b'])

    def testDuplicateOutputsOK(self):
        """Test for duplicate output files that are not duplicates due to prereqs."""
        out = self.runTests('-i', 'duplicate_outputs_ok').output
        out += self.runTests('-i', 'duplicate_outputs_ok', '--heavy').output

        # skip case
        self.assertNotRegex(out, 'skipped_out.e')
        # heavy case
        self.assertNotRegex(out, 'heavy_out.e')
        # all
        self.assertNotRegex(out, 'FATAL TEST HARNESS ERROR')
