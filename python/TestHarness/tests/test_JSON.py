import subprocess, os
from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testJSON(self):
    	with self.assertRaises(subprocess.CalledProcessError) as cm:
    		self.runTests('-i', 'json')
    	self.assertIn('python/testoutput.json', cm.filenameg)