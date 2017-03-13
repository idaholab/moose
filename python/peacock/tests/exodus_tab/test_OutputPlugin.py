#!/usr/bin/env python
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
        self._widget, self._window = main(size=[600,600])
        self._widget.initialize([self._filename])

    def testPython(self):
        """
        Tesgit t script writer.
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
        differ = mooseutils.ImageDiffer(os.path.join('gold', imagename), imagename)
        print differ.message()
        self.assertFalse(differ.fail())

    def testPNG(self):
        """
        Tests the PNG button is working.
        """
        imagename = self._repr_script.replace('.py', '.png')
        self._widget.OutputPlugin.write.emit(imagename)
        differ = mooseutils.ImageDiffer(os.path.join('gold', imagename), imagename)
        print differ.message()
        self.assertFalse(differ.fail())

    def testLiveScript(self):
        """
        Tests that live script widget.
        """
        self._widget.OutputPlugin.LiveScriptButton.clicked.emit()
        self.assertTrue(self._widget.OutputPlugin.LiveScript.isVisible())
        self.assertIn("chigger.exodus.ExodusReader", self._widget.OutputPlugin.LiveScript.toPlainText())
        self._widget.OutputPlugin._result.setOption('variable', 'convected')
        self._window.onWindowRequiresUpdate()
        self._widget.OutputPlugin.onWindowUpdated()
        self.assertIn("variable='convected'", self._widget.OutputPlugin.LiveScript.toPlainText())


if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
