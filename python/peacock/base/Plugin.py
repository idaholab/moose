from PyQt5 import QtWidgets
from MooseWidget import MooseWidget
import mooseutils
class Plugin(MooseWidget):
    """
    A base class for all plugin objects.

    A plugin object are stand-alone widgets contained by a peacock tab. In general, the plugins
    should be independent and be able to be removed or added to a given tab. Plugins are stored
    Manager objects.

    see Manager.py
    """

    def __init__(self):
        super(Plugin, self).__init__()

        # Name of layout that this plugin should be added (see PluginManager.py)
        self._main_layout_name = 'MainLayout'

        # Widget is disabled until initialize is called
        self.setEnabled(False)

        # The default size policy
        self.setSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Fixed)

        # Must be a QtWidget
        if not isinstance(self, QtWidgets.QWidget):
            mooseutils.mooseError("A Plugin must also be a QWidget.")
            return

        # The Peacock tab index
        self._index = None

    @staticmethod
    def commandLineArgs(parser):
        """
        Allows the plugin to add command line options to the parser.
        """

    def connect(self, other):
        """
        Connect the slots of supplied plugin (other) to the signals emited by this (self) plugin.

        Args:
            other[Plugin]: A plugin object to connect.
        """
        if self is not other:
            for name, signal in self.signals().iteritems():
                slot_name = 'on' + name[0].upper() + name[1:]
                if hasattr(other, slot_name):
                    mooseutils.mooseDebug('{}.{} --> {}.{}'.format(self.__class__.__name__, name, other.__class__.__name__, slot_name))
                    signal.connect(getattr(other, slot_name))

    def initialize(self, *args, **kwargs):
        """
        This method is called by the Manager after all plugins have been created.
        """
        self.setEnabled(True)

    def setMainLayoutName(self, name):
        """
        Method for changing the name of the main layout that this plugin will be added.

        Args:
            name[str]: The name of the layout within the PluginManager.
        """
        self._main_layout_name = name

    def mainLayoutName(self):
        """
        Return the name of the layout within the PluginManager that this plugin is to be added.
        """
        return self._main_layout_name

    def repr(self):
        """
        Return data for reproducing the plugin as a script.
        """
        return dict()

    def canClose(self):
        """
        Called when the application wants to close.
        This is intended to allow the plugin to check if it has unsaved state and ask
        the user if they want to cancel the close or throw away the changes.
        Return:
            bool: Whether it is OK to close
        """
        return True

    def closing(self):
        """
        Called when the application is about to close.
        This is intended to allow the plugin to do any cleanup before closing
        """
        pass

    def settingsWidget(self):
        """
        Returns a widget to be included in the global settings widget
        """
        return None

    def clearRecentlyUsed(self):
        """
        Clears any recently used items
        """

    def addToMainMenu(self, menubar):
        """
        This allows the plugin to add menu items to the main menu.
        Args:
            menubar[QMenuBar]: Menubar to add items to
        """
        pass

    def onCurrentChanged(self, index):
        """
        Executes when the TabWidget (TabPluginManager) changes active tabs.

        Inputs:
            index[int]: The index of the active tab
        """
        pass

    def setTabIndex(self, index, signal=None):
        """
        Set the Peacock Tab index (see TabPluginManager)
        """
        if signal:
            signal.connect(self.onCurrentChanged)
        self._index = index
