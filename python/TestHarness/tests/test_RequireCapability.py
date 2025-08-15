#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarnessTestCase import TestHarnessTestCase
from TestHarness import TestHarness
from TestHarness.StatusSystem import StatusSystem

import unittest

class TestRequireCapability(TestHarnessTestCase):
    def testTestHarnessBuildRequiredCapabilities(self):
        """
        Tests TestHarness.buildRequiredCapabilities()
        """
        # Not registered
        with self.assertRaisesRegex(SystemExit, 'Required capability "bar" is not registered'):
            TestHarness.buildRequiredCapabilities(['foo'], ['bar'])

        # True value -> false augmented value
        res = TestHarness.buildRequiredCapabilities(['foo'], ['foo'])
        self.assertEqual(res, [('foo', False)])

        # False value -> true augmented value
        res = TestHarness.buildRequiredCapabilities(['foo'], ['!foo'])
        self.assertEqual(res, [('foo', True)])

        # Has stripping
        res = TestHarness.buildRequiredCapabilities(['foo'], ['  foo '])
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0][0], 'foo')

        # Multiple
        res = TestHarness.buildRequiredCapabilities(['foo', 'bar'], ['foo', '!bar'])
        self.assertEqual(len(res), 2)
        self.assertEqual(res[0], ('foo', False))
        self.assertEqual(res[1], ('bar', True))

    def testTestHarnessOptions(self):
        """
        Test that the test harness will set the _required_capabilities option
        """
        # No capabilities set, can't run
        with self.assertRaisesRegex(SystemExit, 'Cannot use --require-capability with --no-capabilities'):
            self.runTests('--require-capability', 'petsc', no_capabilities=True)

        # Not registered
        with self.assertRaisesRegex(SystemExit, 'Required capability "foo" is not registered'):
            self.runTests('--require-capability', 'foo', no_capabilities=False)

        # Is registered and is set
        res = self.runTests(
            '--require-capability',
            'moosetestapp',
            no_capabilities=False,
            run=False
        )
        self.assertEqual(res.harness.options._required_capabilities, [('moosetestapp', False)])

        # Multiple
        res = self.runTests(
            '--require-capability',
            'moosetestapp',
            '--require-capability',
            '!compiler',
            no_capabilities=False,
            run=False
        )
        self.assertEqual(
            res.harness.options._required_capabilities,
            [('moosetestapp', False), ('compiler', True)]
        )

    def testTesterSkip(self):
        """
        Test the Tester skipping tests based on --required-capabilities
        """
        def run_test(skip, require_capabilities, test_capabilities=None):
            test_name = 'test'
            tests = {
                test_name: {
                    'type': 'RunApp',
                    'input': 'unused',
                    'should_execute': False,
                }
            }
            if test_capabilities:
                tests[test_name]['capabilities'] = f"'{test_capabilities}'"
            args = []
            for v in require_capabilities:
                args += ['--require-capability', v]
            res = self.runTests(*args, tests=tests, no_capabilities=False)
            job = self.getJobWithName(res.harness, test_name)
            if skip:
                self.assertEqual(job.getStatus(), StatusSystem.skip)
                self.assertEqual(job.getCaveats(), set(['Missing required capabilities']))
            else:
                self.assertEqual(job.getStatus(), StatusSystem.finished)

        # Capability is not in the test spec capabilities
        run_test(True, ['moosetestapp'])
        run_test(True, ['moosetestapp'], 'compiler')

        # Capability is in the test spec capabilities
        run_test(False, ['moosetestapp'], 'moosetestapp')

        # Test spec has complex capabilities and still runs
        run_test(False, ['moosetestapp'], 'moosetestapp & compiler')

        # Multiple --require-capability
        run_test(False, ['moosetestapp', 'compiler'], 'compiler & moosetestapp')
        run_test(False, ['moosetestapp', 'compiler'], 'compiler & method & moosetestapp')

if __name__ == '__main__':
    unittest.main()
