#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.utils.ExeLauncher import runExe
from peacock.PeacockException import FileExistsException, BadExecutableException
from peacock.utils import Testing
from PyQt5 import QtWidgets

class Tests(Testing.PeacockTester):
    qapp = QtWidgets.QApplication([])

    def test_runexe(self):
        with self.assertRaises(FileExistsException):
            runExe("/no_exist.exe", "")

        out = runExe("/bin/echo", "output")
        self.assertEqual(out, "output\n")

        out = runExe("/bin/echo", ["output"])
        self.assertEqual(out, "output\n")

        with self.assertRaises(BadExecutableException):
            runExe("false", "")


if __name__ == '__main__':
    Testing.run_tests()
