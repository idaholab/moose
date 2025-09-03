#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import unittest
from unittest.mock import patch
from io import StringIO
from mooseutils import message

class TestMooseMessage(unittest.TestCase):
    """
    Tests the usage of the various messages functions in message package.
    """

    def setUp(self):
        self._stdout_patcher = patch("sys.stdout", new=StringIO())
        self.mock_stdout = self._stdout_patcher.start()
        self._stderr_patcher = patch("sys.stderr", new=StringIO())
        self.mock_stderr = self._stderr_patcher.start()

    def tearDown(self):
        self._stdout_patcher.stop()
        self._stderr_patcher.stop()

    def testMooseMessageDefault(self):
        """
        Test the default message with a string and a number supplied.
        """
        message.mooseMessage("The default message with a number", 1.0)
        output = self.mock_stdout.getvalue()
        self.assertIn("The default message with a number 1.0", output)

    @unittest.skip('Breaks with current package')
    def testMooseMessageTraceback(self):
        """
        Test that the traceback argument is operational.
        """
        message.mooseMessage("A message", "with a traceback!", traceback = True)
        output = self.mock_stdout.getvalue()
        err = self.mock_stderr.getvalue()
        self.assertIn("A message with a traceback!", output)
        self.assertIn("message.mooseMessage", err)

    def testMooseMessageColor(self):
        """
        Test that the color flag is working.
        """
        message.mooseMessage("This should be RED.", color = 'RED')
        output = self.mock_stdout.getvalue()
        self.assertIn('\033[31m', output)

    def testMooseMessageDebugOn(self):
        """
        Test that the debug flag enables debug messages.
        """
        with patch("mooseutils.message.MOOSE_DEBUG_MODE", True):
            message.mooseMessage("You should see this!", debug=True)
        output = self.mock_stdout.getvalue()
        self.assertIn("You should see this!", output)

    def testMooseMessageDebugOff(self):
        """
        Test that the debug flag enables debug messages.
        """
        with patch("mooseutils.message.MOOSE_DEBUG_MODE", False):
            message.mooseDebug("You should see this!", debug=True)
        output = self.mock_stdout.getvalue()
        self.assertIn("You should see this!", output)

    @unittest.skip('Breaks with current package')
    def testMooseError(self):
        """
        Tests mooseError function.
        """
        message.mooseError("Don't do it!")
        output = self.mock_stdout.getvalue()
        err = self.mock_stderr.getvalue()
        self.assertIn('ERROR', output)
        self.assertIn("Don't do it!", output)
        self.assertIn("in mooseError", err)
        self.assertIn('\033[31m', output)

    def testMooseWarning(self):
        """
        Tests mooseWarning function.
        """
        message.mooseWarning("Just a little warning")
        output = self.mock_stdout.getvalue()
        self.assertIn('WARNING', output)
        self.assertIn("Just a little warning", output)
        self.assertIn('\033[33m', output)

    def testDebugMessageOn(self):
        """
        Test use of mooseDebug function, with debugging enabled.
        """
        with patch("mooseutils.message.MOOSE_DEBUG_MODE", True):
            message.mooseDebug("You should see this!")
        output = self.mock_stdout.getvalue()
        self.assertIn("You should see this!", output)

    def testDebugMessageOff(self):
        """
        Test use of mooseDebug function, with debugging disabled.
        """
        with patch("mooseutils.message.MOOSE_DEBUG_MODE", False):
            message.mooseDebug("You should NOT see this!")
        output = self.mock_stdout.getvalue()
        self.assertNotIn("You should NOT see this!", output)



if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True, exit=False)
