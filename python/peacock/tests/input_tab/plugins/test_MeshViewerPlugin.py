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

from PyQt5 import QtWidgets

from peacock.Input.plugins.MeshViewerPlugin import main
from peacock.utils import Testing

class TestMeshViewerPlugin(Testing.PeacockImageTestCase):
    """
    Testing for MeshViewerPlugin
    """

    #: QApplication: The main App for QT, this must be static to work correctly.
    qapp = QtWidgets.QApplication(sys.argv)

    #: str: The filename to load.
    _filename = Testing.get_chigger_input('mesh_only.e')

    def setUp(self):
        """
        Loads an Exodus file in the VTKWindowWidget object using a structure similar to the ExodusViewer widget.
        """
        self.sleepIfSlow()
        self._widget, self._window = main(size=[600,600])
        self._window.onSetFilename(self._filename)
        self._window.onWindowRequiresUpdate()

    def testHighlight(self):
        """
        Test the highlighting is working.
        """
        self._window.onHighlight(block=['1'])
        self.assertImage('testHighlightOn.png', allowed=0.98)

        self._window.onHighlight()
        self.assertImage('testHighlightOff.png')

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
