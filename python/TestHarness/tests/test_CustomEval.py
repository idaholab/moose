
from TestHarnessTestCase import TestHarnessTestCase
import subprocess
class TestHarnessTester(TestHarnessTestCase):
	def testCustomEval(self):
		with self.assertRaises(subprocess.CalledProcessError) as cm:
			self.runTests('-i', 'custom_eval')		
		e = cm.exception
		self.assertIn('Custom evaluation failed.', e.output.decode('utf-8')
)