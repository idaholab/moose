#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5 import QtWidgets
from .MooseWidget import MooseWidget
import mooseutils
from .Preferences import Preferences

class Plugin(MooseWidget):
    """
    A base class for all plugin objects.

    A plugin object are stand-alone widgets contained by a peacock tab. In general, the plugins
    should be independent and be able to be removed or added to a given tab. Plugins are stored
    Manager objects.

    see Manager.py
    """

    def __init__(self, layout='MainLayout', settings_key="", **kwargs):
        super(Plugin, self).__init__()

        # Name of layout that this plugin should be added (see PluginManager.py)
        self._main_layout_name = layout

        # The default size policy
        self.setSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Fixed)

        # Must be a QtWidget
        if not isinstance(self, QtWidgets.QWidget):
            mooseutils.mooseError("A Plugin must also be a QWidget.")
            return

        # The Peacock tab index
        self._index = None
        self._plugin_manager = None
        self._preferences = Preferences(settings_key)

    @staticmethod
    def commandLineArgs(parser):
        """
        Allows the plugin to add command line options to the parser.
        """

    def setup(self):
        """
        Adds automatic Preference callback connection to the setup method.
        """
        super(Plugin, self).setup()

        for key, widget in self._preferences._widgets.iteritems():
            name = key.split('/')[-1]
            name = '_prefCallback{}{}'.format(name[0].upper(), name[1:])
            callback = getattr(self, name, None)
            if callback:
                widget.valueSaved.connect(callback)

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
                    mooseutils.mooseDebug('{}.{} --> {}.{}'.format(self.__class__.__name__, name,
                                                                   other.__class__.__name__, slot_name))
                    signal.connect(getattr(other, slot_name))

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

    def onPreferencesSaved(self):
        """
        Called when the preferences have been saved.
        """

    def clearRecentlyUsed(self):
        """
        Clears any recently used items
        """

    def addToMenu(self, menu):
        pass

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

    def preferenceWidgets(self):
        return self._preferences.widgets()

    def setupMenu(self, menu):
        pass
