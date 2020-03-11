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
from PyQt5 import QtCore, QtWidgets
import unittest
import peacock


class Plugin0(QtWidgets.QWidget, peacock.base.Plugin):
    signal = QtCore.pyqtSignal()
    def __init__(self):
        super(Plugin0, self).__init__()
        self.setup()
        self._initialize = False

class Plugin1(QtWidgets.QWidget, peacock.base.Plugin):
    def __init__(self):
        super(Plugin1, self).__init__()
        self.setup()
        self._initialize = False
    def onSignal(self):
        self._signal_emitted = True

class Manager(QtWidgets.QWidget, peacock.base.PluginManager):
    def __init__(self):
        super(Manager, self).__init__(plugins=[Plugin0, Plugin1])
        self.MainLayout = QtWidgets.QHBoxLayout()
        self.setLayout(self.MainLayout)
        self.setup()


class TestPluginManger(unittest.TestCase):
    """
    Test class for PluginManager.
    """

    #: QApplication: The main App for QT, this must be static to work correctly.
    qapp = QtWidgets.QApplication(sys.argv)

    def setUp(self):
        """
        Create plugins to connected via manager.
        """
        self._manager = Manager()

    def testConnect(self):
        """
        Test that signal and slots are connected.
        """
        self._manager.Plugin0.signal.emit()
        self.assertTrue(self._manager.Plugin1._signal_emitted)

if __name__ == '__main__':
    import unittest
    unittest.main(module=__name__, verbosity=2)
