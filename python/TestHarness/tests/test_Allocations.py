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
    def testSkippedAllocations(self):
        """
        Scenarios which trigger skipped tests due to insufficient
        resource allocation.
        """
        # Subject a normally passing test to impossible cpu allocations
        output = self.runTests('--no-color', '-i', 'always_ok', '-p', '2', '-j', '1')
        self.assertRegex(output.decode('utf-8'), 'tests/test_harness.always_ok.*? \[INSUFFICIENT SLOTS\] SKIP')

        # Subject a normally passing test to impossible thread allocations
        output = self.runTests('--no-color', '-i', 'always_ok', '--n-threads', '2', '-j', '1')
        self.assertRegex(output.decode('utf-8'), 'tests/test_harness.always_ok.*? \[INSUFFICIENT SLOTS\] SKIP')

        # A combination of threads*cpus with too low a hard limit (3*3= -j9)
        output = self.runTests('--no-color', '-i', 'allocation_test', '--n-threads', '3', '-p', '3', '-j', '8')
        self.assertRegex(output.decode('utf-8'), 'tests/test_harness.allocation_test.*? \[INSUFFICIENT SLOTS\] SKIP')

    def testOversizedCaveat(self):
        """
        Scenarios which trigger only the 'oversized' caveat.
        """
        # A test which has no min/max cpu parameters should print oversized
        # when subjected to -p 2
        output = self.runTests('-i', 'always_ok', '-p', '2').decode('utf-8')
        self.assertNotIn('CPUS', output)
        self.assertIn('OVERSIZED', output)

        # A test which has no min/max thread parameters should print oversized
        # when subjected to --n-threads 2
        output = self.runTests('-i', 'always_ok', '--n-threads', '2').decode('utf-8')
        self.assertNotIn('THREADS', output)
        self.assertIn('OVERSIZED', output)

    def testCpuCaveats(self):
        """
        Scenarios which trigger the min/max CPU caveat.
        Note: --n-threads is present to suppress the threading
              caveat for more accurate caveat detection.
        """
        # Test MIN CPUs / Oversized caveat using soft limit (no -j) on a test
        # having a minimum cpu parameter of 2.
        output = self.runTests('-i', 'allocation_test', '--n-threads', '2').decode('utf-8')
        self.assertNotIn('MIN_THREADS', output)
        self.assertIn('MIN_CPUS=2', output)
        self.assertIn('OVERSIZED', output)

        # Test MAX CPUs / Oversized caveat on a test having a maximum cpu
        # parameter of 3 (and we subjected it to 4).
        output = self.runTests('-i', 'allocation_test', '-p', '4', '--n-threads', '2').decode('utf-8')
        self.assertNotIn('MIN_THREADS', output)
        self.assertIn('MAX_CPUS=3', output)
        self.assertIn('OVERSIZED', output)

    def testThreadCaveats(self):
        """
        Scenarios which trigger the min/max threading caveat.
        Note: -j/p is present to suppress the min/max cpu oversize
              caveat for more accurate caveat detection.
        """
        # MIN Threads caveat
        # Note: 1*2 should be -j 2 but the test minimum is 2 threads, so we need
        # to use -j 4 to suppress any cpu caveats. Oversized will not trigger as
        # -j4 satisfies this test's requirements.
        output = self.runTests('-i', 'allocation_test', '-j', '4', '-p', '2', '--n-threads', '1').decode('utf-8')
        self.assertNotIn('CPUS', output)
        self.assertNotIn('OVERSIZED', output)
        self.assertIn('MIN_THREADS=2', output)

        # MAX Threads caveat
        # Note: 2*4 should be -j 8 but the test maximum is 3 threads, so we
        # are specifically testing that setting a lower j does _not_ trigger an
        # insufficient skipped test scenario. Oversized will not trigger as
        # -j6 satisfies this test's requirements.
        output = self.runTests('-i', 'allocation_test', '-j', '6', '-p', '2', '--n-threads', '4').decode('utf-8')
        self.assertNotIn('CPUS', output)
        self.assertNotIn('OVERSIZED', output)
        self.assertIn('MAX_THREADS=3', output)

    def testPerfectAllocation(self):
        """
        Scenario which trigger no caveats.
        """
        # Passing test triggering no caveats, as supplied allocations satisfies
        # the test's requirements
        output = self.runTests('-i', 'allocation_test', '-j', '4', '-p', '2', '--n-threads', '2').decode('utf-8')
        self.assertNotIn('MIN_THREADS', output)
        self.assertNotIn('MAX_THREADS', output)
        self.assertNotIn('MIN_CPUS', output)
        self.assertNotIn('MAX_CPUS', output)
        self.assertNotIn('OVERSIZED', output)
        self.assertRegex(output, 'tests/test_harness.allocation_test.*OK')
