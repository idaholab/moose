
from TestHarnessTestCase import TestHarnessTestCase
import subprocess
class TestHarnessTester(TestHarnessTestCase):
  def testCustomEval(self):
    with self.assertRaises(subprocess.CalledProcessError) as cm:
      self.runTests('-i', 'custom_eval')
    e = cm.exception
    self.assertIn('Custom evaluation failed', e.output)

    #test expect out failure
    with self.assertRaises(subprocess.CalledProcessError) as cm:
      self.runTests('-i', 'custom_eval')
    e = cm.exception
    self.assertIn('expect_out and absent_out can not be supplied', e.output)
