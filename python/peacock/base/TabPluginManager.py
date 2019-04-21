#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import argparse
from PyQt5 import QtWidgets
from .PluginManager import PluginManager
from .TabPlugin import TabPlugin
from .TabbedPreferences import TabbedPreferences

class TabPluginManager(QtWidgets.QTabWidget, PluginManager):
    """
    General class for building a main application from TabPlugin objects.

    Args:
        plugins[list]: A list of TabPlugin classes to instantiate for this widget.
        plugin_base[class]: (Default:TabPlugin) The type of plugin that this manager is allowed to instatiate.
    """

    def __init__(self, plugins=[], plugin_base=TabPlugin):
        super(TabPluginManager, self).__init__(plugins=plugins, plugin_base=plugin_base)
        self._description = 'A GUI application (use setDescription within __init__ to change this message).'
        self.setup()
        self._pref_widget = None

    def description(self):
        """
        Return the command-line help description.
        """
        return self._description

    def setDescription(self, description):
        """
        Set the command-line help description.

        Args:
            description[str]: The text to appear when application is executed with -h.
        """
        self._description = description

    def addObject(self, widget):
        """
        Method for adding a widget to the Manager. (override)

        Args:
            widget[QWidget]: The widget to add a new tab.
        """
        index = self.addTab(widget, widget.tabName())
        widget.setTabIndex(index, self.currentChanged)

    def parse(self):
        """
        Parses the command-line arguments from the TabPlugin objects.
        """
        parser = argparse.ArgumentParser(description=self.description())
        for plugin_class in self._plugin_classes:
            plugin_class.commandLineArgs(parser)

    def initialize(self, options):
        """
        Initialize the TabPlugin objects with the command-line options.

        Input:
            options: Command-line arguments from argparse
        """
        for tab_plugin in self._all_plugins:
            tab_plugin.initialize(options)

    def tabPreferenceWidget(self):
        """
        Returns an instance of a widget to set preferences
        """
        if not self._pref_widget:
            self._pref_widget = TabbedPreferences(self._all_plugins)
        return self._pref_widget


if __name__ == '__main__':
    import sys
    from peacock.ExodusViewer.ExodusViewer import ExodusViewer
    from peacock.PostprocessorViewer.PostprocessorViewer import PostprocessorViewer
    from peacock.PostprocessorViewer.VectorPostprocessorViewer import VectorPostprocessorViewer
    app = QtWidgets.QApplication(sys.argv)
    main = TabPluginManager(plugins=[ExodusViewer, PostprocessorViewer, VectorPostprocessorViewer])
    main.show()
    main.initialize()
    sys.exit(app.exec_())
