import os
import unittest
import subprocess
from TestHarnessTestCase import TestHarnessTestCase

class TestCyclicDependency(TestHarnessTestCase):
  """
  Test cyclic dependency error.
  """

  def testCyclic(self):
    with self.assertRaises(subprocess.CalledProcessError) as cm:
      self.runTests('-i', 'cyclic_tests')

    e = cm.exception
    self.assertEqual(e.returncode, 1)
    self.assertIn('Cyclic or Invalid Dependency Detected!', e.output)
    self.assertIn('tests/test_harness.testA', e.output)
    self.assertIn('tests/test_harness.testB', e.output)
