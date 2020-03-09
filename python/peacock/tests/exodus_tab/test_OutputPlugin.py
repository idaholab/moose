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
import os
import unittest
import subprocess
from PyQt5 import QtWidgets
import mooseutils
from peacock.ExodusViewer.plugins.OutputPlugin import main
from peacock.utils import Testing

class TestOutputPlugin(Testing.PeacockImageTestCase):
    """
    Testing for VTKWindowPlugin
    """

    #: QApplication: The main App for QT, this must be static to work correctly.
    qapp = QtWidgets.QApplication(sys.argv)

    #: str: The filename to load.
    _filename = Testing.get_chigger_input('mug_blocks_out.e')

    #: str: The script file created for testing
    _repr_script = 'TestOutputPlugin_repr.py'

    #: str: Temporary filename for testing delayed load (see testFilename)
    _temp_file = 'TestOutputPlugin_test.e'

    @classmethod
    def setUpClass(cls):
        super(TestOutputPlugin, cls).setUpClass()

        for filename in [cls._repr_script, cls._temp_file]:
            if os.path.exists(filename):
                os.remove(filename)

    def setUp(self):
        """
        Loads an Exodus file in the VTKWindowWidget object using a structure similar to the ExodusViewer widget.
        """
        self._filename = Testing.get_chigger_input('mug_blocks_out.e')
        self._widget, self._window, self._main = main(size=[600,600])
        self._window.onSetFilename(self._filename)
        self._window.onSetVariable('diffused')
        self._window.onWindowRequiresUpdate()

    def testPython(self):
        """
        Test script writer.
        """

        # Test that the script is created
        imagename = self._repr_script.replace('.py', '.png')
        self._window.onResultOptionsChanged({'variable':'diffused'})
        self._window.onWindowRequiresUpdate()
        self._window._window.setOptions(test=True)
        self._widget.OutputPlugin.write.emit(self._repr_script)
        self.assertTrue(os.path.exists(self._repr_script))

        # Inject write command
        with open(self._repr_script, 'a') as fid:
            fid.write('\nwindow.write({})'.format(repr(imagename)))

        # Execute the script
        subprocess.call(['python', self._repr_script], stdout=open(os.devnull, 'wb'), stderr=subprocess.STDOUT)
        self.assertTrue(os.path.exists(imagename))

        # Diff the image from the script
        if self._window.devicePixelRatio() == 1:
            differ = mooseutils.ImageDiffer(os.path.join('gold', imagename), imagename)
            print(differ.message())
            self.assertFalse(differ.fail())

    def testPNG(self):
        """
        Tests the PNG button is working.
        """
        imagename = self._repr_script.replace('.py', '.png')
        self._window.onResultOptionsChanged({'variable':'diffused'})
        self._widget.OutputPlugin.write.emit(imagename)
        differ = mooseutils.ImageDiffer(os.path.join('gold', imagename), imagename)
        print(differ.message())
        self.assertFalse(differ.fail())

    def testLiveScript(self):
        """
        Tests that live script widget.
        """
        self._widget.OutputPlugin.LiveScript.setChecked(True)
        self._widget.OutputPlugin.LiveScript.toggled.emit(True)
        self.assertTrue(self._widget.OutputPlugin.LiveScriptWindow.isVisible())
        self.assertIn("chigger.exodus.ExodusReader", self._widget.OutputPlugin.LiveScriptWindow.toPlainText())
        self.assertIn("variable='diffused'", self._widget.OutputPlugin.LiveScriptWindow.toPlainText())


if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
