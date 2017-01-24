import os
import unittest
import subprocess
from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
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
    self.assertIn('tests/test_harness.testC', e.output)


  """
  Test skipping a test if its prereq is also skipped
  """
  def testDependencySkip(self):
    output = self.runTests('-i', 'depend_skip_tests')

    self.assertIn('skipped (always skipped)', output)
    self.assertIn('skipped (skipped dependency)', output)


  """
  Test for RUNNING status in the TestHarness
  """
  def testLongRunningStatus(self):
    output = self.runTests('-i', 'long_running')

    self.assertIn('RUNNING...', output)
    self.assertIn('[FINISHED]', output)


  """
  Test for Exodiffs, CSVDiffs
  """
  def testDiffs(self):
    with self.assertRaises(subprocess.CalledProcessError) as cm:
      self.runTests('-i', 'diffs')

    e = cm.exception
    self.assertRegexpMatches(e.output, 'test_harness\.exodiff.*?FAILED \(EXODIFF\)')
    self.assertRegexpMatches(e.output, 'test_harness\.csvdiff.*?FAILED \(CSVDIFF\)')


  """
  Test for Missing Gold
  """
  def testMissingGold(self):
    with self.assertRaises(subprocess.CalledProcessError) as cm:
      self.runTests('-i', 'missing_gold')

    e = cm.exception
    self.assertRegexpMatches(e.output, 'test_harness\.exodiff.*?FAILED \(MISSING GOLD FILE\)')


  """
  Test for Expect Err/Out
  """
  def testExpect(self):
    with self.assertRaises(subprocess.CalledProcessError) as cm:
      self.runTests('-i', 'expect')

    e = cm.exception
    self.assertRegexpMatches(e.output, 'test_harness\.no_expect_err.*?FAILED \(NO EXPECTED ERR\)')
    self.assertRegexpMatches(e.output, 'test_harness\.no_expect_out.*?FAILED \(EXPECTED OUTPUT MISSING\)')
