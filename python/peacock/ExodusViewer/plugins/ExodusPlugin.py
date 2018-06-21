#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import peacock
from PyQt5 import QtWidgets

class ExodusPlugin(peacock.base.Plugin):
    """
    Plugin class for the Exodus volume rendering portion of Peacock.
    """

    def __init__(self, layout='LeftLayout', settings_key=""):
        super(ExodusPlugin, self).__init__(layout=layout, settings_key=settings_key)

        # The default layout name
        self.setSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.MinimumExpanding)

        # Ubiquitous member variables
        self._variable = None
        self._filename = None
        self._reader = None
        self._result = None
        self._window = None

        # Disable the widget (the onWindowCreated slot will enable)
        self.setEnabled(False)

    def onPlayStart(self):
        """
        Disables the plugin when playing begins.
        """
        self.setEnabled(False)

    def onPlayStop(self):
        """
        Enables widget when the playing stops.
        """
        self.setEnabled(True)

    def onFileChanged(self, filename):
        """
        Stores filename in the plugin.

        Args:
            filename[str]: The name of the current file being viewed.
        """
        self._filename = filename

    def onVariableChanged(self, variable):
        """
        Stores the current variable.

        Args:
            variable[str]: The name of the current variable being viewed.
        """
        self._variable = variable

    def onWindowCreated(self, reader, result, window):
        """
        Stores the created chigger objects for use by the plugin.

        Args:
            reader[chigger.ExodusReader]: The exodus file reader.
            result[chigger.ExodusResult]: The result renderer.
        """
        self.setEnabled(True)
        self._reader = reader
        self._result = result
        self._window = window

    def onWindowReset(self):
        """
        Clears the stored data and disables the widget
        """
        self.setEnabled(False)
        self._reader = None
        self._result = None
        self._window = None

    def stateKey(self, other=""):
        """
        Generates a (mostly) unique key for use in saving state of a widget.
        """
        s = self.__class__.__name__
        if self._filename:
            s += "_" + self._filename
        s += str(other)
        return s

    def onPreFileChanged(self):
        """
        Save the state of the widget before the file changes
        """
        if self.isEnabled():
            self.store(self.stateKey(), 'Filename')

    def onPostFileChanged(self):
        """
        Load the state of the widget based on the new file name.
        """
        self.load(self.stateKey(), 'Filename')
