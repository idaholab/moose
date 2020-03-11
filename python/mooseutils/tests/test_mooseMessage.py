#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys
import unittest
from mooseutils import message

class TestMooseMessage(unittest.TestCase):
    """
    Tests the usage of the various messages functions in message package.
    """

    def testMooseMessageDefault(self):
        """
        Test the default message with a string and a number supplied.
        """
        message.mooseMessage("The default message with a number", 1.0)
        output = sys.stdout.getvalue()
        self.assertIn("The default message with a number 1.0", output)

    @unittest.skip('Breaks with current package')
    def testMooseMessageTraceback(self):
        """
        Test that the traceback argument is operational.
        """
        message.mooseMessage("A message", "with a traceback!", traceback = True)
        output = sys.stdout.getvalue()
        err = sys.stderr.getvalue()
        self.assertIn("A message with a traceback!", output)
        self.assertIn("message.mooseMessage", err)

    def testMooseMessageColor(self):
        """
        Test that the color flag is working.
        """
        message.mooseMessage("This should be RED.", color = 'RED')
        output = sys.stdout.getvalue()
        self.assertIn('\033[31m', output)

    def testMooseMessageDebugOn(self):
        """
        Test that the debug flag enables debug messages.
        """
        message.MOOSE_DEBUG_MODE = True
        message.mooseMessage("You should see this!", debug=True)
        output = sys.stdout.getvalue()
        self.assertIn("You should see this!", output)

    def testMooseMessageDebugOff(self):
        """
        Test that the debug flag enables debug messages.
        """
        message.MOOSE_DEBUG_MODE = False
        message.mooseDebug("You should see this!", debug=True)
        output = sys.stdout.getvalue()
        self.assertIn("You should see this!", output)

    @unittest.skip('Breaks with current package')
    def testMooseError(self):
        """
        Tests mooseError function.
        """
        message.mooseError("Don't do it!")
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
        message.mooseWarning("Just a little warning")
        output = sys.stdout.getvalue()
        self.assertIn('WARNING', output)
        self.assertIn("Just a little warning", output)
        self.assertIn('\033[33m', output)

    def testDebugMessageOn(self):
        """
        Test use of mooseDebug function, with debugging enabled.
        """
        message.MOOSE_DEBUG_MODE = True
        message.mooseDebug("You should see this!")
        output = sys.stdout.getvalue()
        self.assertIn("You should see this!", output)

    def testDebugMessageOff(self):
        """
        Test use of mooseDebug function, with debugging disabled.
        """
        message.MOOSE_DEBUG_MODE = False
        message.mooseDebug("You should NOT see this!")
        output = sys.stdout.getvalue()
        self.assertNotIn("You should NOT see this!", output)



if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True, exit=False)
