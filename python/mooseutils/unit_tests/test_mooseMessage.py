#!/usr/bin/env python
import sys
import unittest
from mooseutils import tools

class TestMooseMessage(unittest.TestCase):
    """
    Tests the usage of the various messages functions in tools package.
    """

    def testMooseMessageDefault(self):
        """
        Test the default message with a string and a number supplied.
        """
        tools.mooseMessage("The default message with a number", 1.0)
        output = sys.stdout.getvalue()
        self.assertIn("The default message with a number 1.0", output)

    def testMooseMessageTraceback(self):
        """
        Test that the traceback argument is operational.
        """
        tools.mooseMessage("A message", "with a traceback!", traceback = True)
        output = sys.stdout.getvalue()
        err = sys.stderr.getvalue()
        self.assertIn("A message with a traceback!", output)
        self.assertIn("tools.mooseMessage", err)

    def testMooseMessageColor(self):
        """
        Test that the color flag is working.
        """
        tools.mooseMessage("This should be RED.", color = 'RED')
        output = sys.stdout.getvalue()
        self.assertIn('\033[31m', output)

    def testMooseMessageDebugOn(self):
        """
        Test that the debug flag enables debug messages.
        """
        tools.MOOSE_DEBUG_MODE = True
        tools.mooseMessage("You should see this!", debug=True)
        output = sys.stdout.getvalue()
        self.assertIn("You should see this!", output)

    def testMooseMessageDebugOff(self):
        """
        Test that the debug flag enables debug messages.
        """
        tools.MOOSE_DEBUG_MODE = False
        tools.mooseDebug("You should see this!", debug=True)
        output = sys.stdout.getvalue()
        self.assertIn("You should see this!", output)

    def testMooseError(self):
        """
        Tests mooseError function.
        """
        tools.mooseError("Don't do it!")
        output = sys.stdout.getvalue()
        err = sys.stderr.getvalue()
        self.assertIn('ERROR', output)
        self.assertIn("Don't do it!", output)
        self.assertIn("in mooseError", err)
        self.assertIn('\033[31m', output)

    def testMooseWarning(self):
        """
        Tests mooseWarning function.
        """
        tools.mooseWarning("Just a little warning")
        output = sys.stdout.getvalue()
        self.assertIn('WARNING', output)
        self.assertIn("Just a little warning", output)
        self.assertIn('\033[33m', output)

    def testDebugMessageOn(self):
        """
        Test use of mooseDebug function, with debugging enabled.
        """
        tools.MOOSE_DEBUG_MODE = True
        tools.mooseDebug("You should see this!")
        output = sys.stdout.getvalue()
        self.assertIn("You should see this!", output)

    def testDebugMessageOff(self):
        """
        Test use of mooseDebug function, with debugging disabled.
        """
        tools.MOOSE_DEBUG_MODE = False
        tools.mooseDebug("You should NOT see this!")
        output = sys.stdout.getvalue()
        self.assertNotIn("You should NOT see this!", output)



if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True, exit=False)
