#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5.QtWidgets import QMainWindow, QApplication
from PyQt5.QtCore import QSettings
from peacock.base.TabPluginManager import TabPluginManager
from peacock.base.TabPlugin import TabPlugin
from peacock.base.MooseWidget import MooseWidget
from peacock.utils import WidgetUtils
from peacock.LogWidget import LogWidget
import os

class BasePeacockMainWindow(QMainWindow, MooseWidget):
    """
    Base class for peacock based apps.
    This takes care of common command line arguments and
    common windows.
    The real app will need to inherit from this and handle
    any connections between the plugins.
    """
    def __init__(self, plugins=[], plugin_base=TabPlugin):
        super(BasePeacockMainWindow, self).__init__()
        self.setObjectName("BasePeacockMainWindow")
        self.tab_plugin = TabPluginManager(plugins=plugins, plugin_base=plugin_base)
        self.setCentralWidget(self.tab_plugin)
        self.log = LogWidget()
        self._closed = False
        self.settings = None
        self.setup()

    @staticmethod
    def commandLineArgs(parser, plugins):
        """
        Set the command line parameters.
        The plugins argument is intended to be the same that will eventually be
        passed to the constructor. We pass it in here separately because we don't
        want to have to instantiate all the widgets just to see if the command
        line is valid.
        Input:
            parser[argparse.ArgumentParser]: parser instance
            plugins[list]: List of plugin classes that might add command line arguments
        """
        for plugin_class in plugins:
            plugin_class.commandLineArgs(parser)
        parser.add_argument("arguments",
                type=str,
                metavar="N",
                nargs="*",
                help="Input file and executable can be set without the command line switches")
        parser.add_argument('--clear-recently-used',
                dest="clear_recent",
                action='store_true',
                help='Clear recently used items')
        parser.add_argument('--clear-settings',
                dest="clear_settings",
                action='store_true',
                help='Clears all peacock settings.')
        parser.add_argument('-w', '--working-dir',
                dest="working_dir",
                help='Set the initial working directory.')
        parser.add_argument('-d', '--debug', '--dbg',
                dest="debug",
                action='store_true',
                help='Run Peacock in debug mode. Setting the PEACOCK_DEBUG environment variable will automatically set this.')
        group = parser.add_argument_group("Window size", "Startup window size")
        group.add_argument('-size',
                dest='size',
                nargs=2,
                help='Size of the window')
        group.add_argument('-max',
                dest='max',
                action='store_true',
                help='Open in maximized view')
        group.add_argument('-full',
                dest='full',
                action='store_true',
                help='Open in fullscreen view')

    def _showLog(self):
        """
        Toggles showing the log widget
        """
        if self.log.isVisible():
            self.log.hide()
        else:
            self.log.show()

    def _showPreferences(self):
        """
        Shows the preferences window
        """
        if not self.settings:
            self.settings = self.tab_plugin.tabPreferenceWidget()
        self.settings.load()
        self.settings.show()
        self.settings.raise_()

    def _addMenus(self):
        """
        Internal method to allow plugins to add menus to the main menu bar.
        """
        menubar = self.menuBar()
        menubar.setNativeMenuBar(False)
        peacock_menu = menubar.addMenu("Peacock")
        WidgetUtils.addAction(peacock_menu, "View Log", self._showLog, "Ctrl+L", True)
        WidgetUtils.addAction(peacock_menu, "Preferences", self._showPreferences)
        WidgetUtils.addAction(peacock_menu, "E&xit", self.close, "Ctrl+Q", True)
        for name, plugin in self.tab_plugin._plugins.items():
            plugin.addToMainMenu(menubar)

    def closeEvent(self, event):
        """
        Gets called when the application wants to close the main window.
        We allow the plugins to cancel the close event in case they have
        unsaved state.

        Input:
            event[QEvent]: The event object
        """
        # closeEvent can get called multiple times, so this guard
        # just prevents going through all this again.
        if self._closed:
            return
        for name, plugin in self.tab_plugin._plugins.items():
            if not plugin.canClose():
                event.ignore()
                self._closed = False
                return
        self._cleanup()
        event.accept()
        QApplication.quit()
        self._closed = True

    def _cleanup(self):
        """
        Internal mehtod to allow plugins to cleanup before we actually close.
        """
        for name, plugin in self.tab_plugin._plugins.items():
            plugin.closing()

    def _setWindowSize(self, command_line_options):
        """
        Set the window size of the main window
        Input:
            command_line_options[argparse.Namespace]: parsed options given on the command line
        """
        if command_line_options.full:
            self.showFullScreen()
        elif command_line_options.max:
            self.showMaximized()
        elif command_line_options.size:
            size = command_line_options.size
            self.resize(int(size[0]), int(size[1]))

    def _clearSettings(self):
        """
        Just clears all peacock settings.
        """
        settings = QSettings()
        settings.clear()
        settings.sync()

    def initialize(self, command_line_options):
        """
        Initialize the main window
        Input:
            command_line_options[argparse.Namespace]: parsed options given on the command line
        """
        if command_line_options.clear_settings:
            self._clearSettings()

        command_line_options.start_dir = os.getcwd()

        self._addMenus()

        self.tab_plugin.initialize(command_line_options)
        for name, plugin in self.tab_plugin._plugins.items():
            plugin.setEnabled(True)
        self._setWindowSize(command_line_options)
        if command_line_options.debug or os.getenv("PEACOCK_DEBUG"):
            from mooseutils import message
            message.MOOSE_DEBUG_MODE = True

if __name__ == '__main__':
    import sys, argparse
    from peacock.ExodusViewer.ExodusViewer import ExodusViewer
    from peacock.PostprocessorViewer.PostprocessorViewer import PostprocessorViewer
    from peacock.PostprocessorViewer.VectorPostprocessorViewer import VectorPostprocessorViewer
    from peacock.Input.InputFileEditorWithMesh import InputFileEditorWithMesh
    from peacock.Execute.ExecuteTabPlugin import ExecuteTabPlugin
    app = QApplication(sys.argv)
    plugins = [InputFileEditorWithMesh, ExecuteTabPlugin, ExodusViewer, PostprocessorViewer, VectorPostprocessorViewer]
    parser = argparse.ArgumentParser()
    BasePeacockMainWindow.commandLineArgs(parser, plugins)
    main = BasePeacockMainWindow(plugins=plugins)
    main.show()
    main.initialize(parser.parse_args())
    sys.exit(app.exec_())
