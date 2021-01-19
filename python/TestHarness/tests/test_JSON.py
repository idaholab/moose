import subprocess, os, json, unittest
from TestHarnessTestCase import TestHarnessTestCase
class TestHarnessTester(TestHarnessTestCase):
    def testJSON(self):
        output = self.runTests('-i', 'always_ok', '--json')
        print(os.getcwd(), output)
        jsonpath = "../../../test/testoutput.json"
        self.assertTrue(os.path.exists(jsonpath))
        with open(jsonpath) as f:
            json.load(f)
