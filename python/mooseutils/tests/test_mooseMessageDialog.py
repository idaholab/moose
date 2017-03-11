#!/usr/bin/env python
import sys
import unittest
import mooseutils
from PyQt5 import QtWidgets

class TestMooseMessageDialog(unittest.TestCase):
    """
    Tests the usage of the various messages functions in message package.
    """

    app = QtWidgets.QApplication(sys.argv)

    def testMooseMessageDefault(self):
        """
        Test the default dialog message.
        """
        box = mooseutils.mooseMessage("A message", dialog = True, test = True)
        self.assertTrue(box.text() == "A message")
        self.assertTrue(box.icon() == QtWidgets.QMessageBox.NoIcon)

    def testMooseMessageWarning(self):
        """
        Test the warning dialog message.
        """
        box = mooseutils.mooseWarning("A message", dialog = True, test = True)
        self.assertIn("A message", box.text())
        self.assertIn("WARNING", box.text())
        self.assertTrue(box.icon() == QtWidgets.QMessageBox.Warning)

    def testMooseMessageError(self):
        """
        Test the error dialog message.
        """
        box = mooseutils.mooseError("A message", dialog = True, test = True)
        self.assertIn("A message", box.text())
        self.assertIn("ERROR", box.text())
        self.assertTrue(box.icon() == QtWidgets.QMessageBox.Critical)

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True, exit=False)
