import argparse
from PyQt5 import QtWidgets
from PluginManager import PluginManager
from TabPlugin import TabPlugin

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
