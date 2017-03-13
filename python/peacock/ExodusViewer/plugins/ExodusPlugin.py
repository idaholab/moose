import peacock
from PyQt5 import QtWidgets

class ExodusPlugin(peacock.base.Plugin):
    """
    Plugin class for the Exodus volume rendering portion of Peacock.
    """

    def __init__(self):
        super(ExodusPlugin, self).__init__()

        # The default layout name
        self.setMainLayoutName('LeftLayout')
        self.setSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.MinimumExpanding)

        # Ubiquitous member variables
        self._variable = None
        self._filename = None
        self._reader = None
        self._result = None
        self._window = None

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
