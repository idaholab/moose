import os
import unittest
import subprocess

class TestHarnessTestCase(unittest.TestCase):
    """
    TestCase class for running TestHarness commands.
    """

    def runTests(self, *args):
        cmd = ['./run_tests'] + list(args)
        return subprocess.check_output(cmd, cwd=os.path.join(os.getenv('MOOSE_DIR'), 'test'))
