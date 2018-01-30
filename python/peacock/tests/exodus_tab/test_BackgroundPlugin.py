#!/usr/bin/env python
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
from PyQt5 import QtWidgets, QtGui, QtCore
from peacock.ExodusViewer.plugins.BackgroundPlugin import main
from peacock.utils import Testing, qtutils
from mooseutils import message

class TestBackgroundPlugin(Testing.PeacockImageTestCase):
    """
    Testing for MeshControl widget.
    """

    qapp = QtWidgets.QApplication(sys.argv)

    def setUp(self):
        """
        Creates a window attached to FilePlugin widget.
        """
        message.MOOSE_TESTING_MODE = True
        qtutils.setAppInformation("peacock_backgroundplugin")

        settings = QtCore.QSettings()
        settings.clear()
        settings.sync()

    def createWidget(self):
        # The file to open
        self._filename = Testing.get_chigger_input('mug_blocks_out.e')
        self._widget, self._window = main(size=[600,600])
        self._window.onFileChanged(self._filename)
        self._window.onResultOptionsChanged({'variable':'diffused'})
        self._window.onWindowRequiresUpdate()

    def testInitial(self):
        """
        Test the initial state of the widget.
        """
        self.createWidget()
        bottom = self._window._window[0].getVTKRenderer().GetBackground()
        bottom_exact = 0.7058823529411765
        for i in range(3):
            self.assertAlmostEqual(bottom[i], bottom_exact)

        top = self._window._window[0].getVTKRenderer().GetBackground2()
        top_exact = 0.43529411764705883
        for i in range(3):
            self.assertAlmostEqual(top[i], top_exact)

        self.assertImage('testInitial.png')

    def testChangeTop(self):
        """
        Test changing top color.
        """
        self.createWidget()
        self._widget.BackgroundPlugin._top = QtGui.QColor(0,255,0)
        self._widget.BackgroundPlugin.color()
        self.assertImage('testTopColor.png')

    def testChangeBottom(self):
        """
        Test changing bottom color.
        """
        self.createWidget()
        self._widget.BackgroundPlugin._bottom = QtGui.QColor(0,0,255)
        self._widget.BackgroundPlugin.color()
        self.assertImage('testBottomColor.png')

    def testSolidColor(self):
        """
        Test gradient toggle.
        """
        self.createWidget()
        self._widget.BackgroundPlugin.GradientToggle.setChecked(False)
        self.assertEqual(self._widget.BackgroundPlugin.TopLabel.text(), 'Background Color:')
        self._widget.BackgroundPlugin._solid = QtGui.QColor(255,0,0)
        self._widget.BackgroundPlugin.color()
        self.assertImage('testSolidColor.png')

    def testExtents(self):
        """
        Test the extents toggle.
        """
        self.createWidget()
        self._widget.BackgroundPlugin.Extents.setChecked(True)
        self.assertImage('testExtents.png')
        self._widget.BackgroundPlugin.Extents.setChecked(False)
        self.assertImage('testInitial.png')

    def testTopColorPrefs(self):
        """
        Test that the preferences work
        """
        settings = QtCore.QSettings()
        settings.setValue("exodus/gradientTopColor", QtGui.QColor(0, 255, 0).name())
        settings.sync()
        self.createWidget()
        self.assertImage('testTopColor.png')

    def testBottomColorPrefs(self):
        """
        Test that the preferences work
        """
        settings = QtCore.QSettings()
        settings.setValue("exodus/gradientBottomColor", QtGui.QColor(0, 0, 255).name())
        settings.sync()
        self.createWidget()
        self.assertImage('testBottomColor.png')

    def testSolidColorPrefs(self):
        """
        Test that the preferences work
        """
        settings = QtCore.QSettings()
        settings.setValue("exodus/solidBackgroundColor", QtGui.QColor(255, 0, 0).name())
        settings.setValue("exodus/backgroundGradient", False)
        settings.sync()
        self.createWidget()
        self.assertImage('testSolidColor.png')

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
