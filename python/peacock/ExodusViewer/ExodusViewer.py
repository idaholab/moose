#!/usr/bin/env python2
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import peacock
from .ExodusPluginManager import ExodusPluginManager
from .plugins.VTKWindowPlugin import VTKWindowPlugin
from .plugins.FilePlugin import FilePlugin
from .plugins.GoldDiffPlugin import GoldDiffPlugin
from .plugins.ColorbarPlugin import ColorbarPlugin
from .plugins.MeshPlugin import MeshPlugin
from .plugins.BackgroundPlugin import BackgroundPlugin
from .plugins.ClipPlugin import ClipPlugin
from .plugins.ContourPlugin import ContourPlugin
from .plugins.OutputPlugin import OutputPlugin
from .plugins.CameraPlugin import CameraPlugin
from .plugins.MediaControlPlugin import MediaControlPlugin
from .plugins.BlockPlugin import BlockPlugin

class ExodusViewer(peacock.base.ViewerBase):
    """
    Widget for viewing ExodusII results.
    """

    @staticmethod
    def commandLineArgs(parser):
        parser.add_argument('--exodus', '-r', nargs='*', default=[], help="A list of ExodusII files to open.")

    def __init__(self, plugins = [VTKWindowPlugin, FilePlugin, BlockPlugin, MediaControlPlugin, GoldDiffPlugin,
                                  ColorbarPlugin, MeshPlugin, ClipPlugin, ContourPlugin, CameraPlugin,
                                  BackgroundPlugin, OutputPlugin]):
        self._exodus_menu = None
        super(ExodusViewer, self).__init__(manager=ExodusPluginManager, plugins=plugins)

    def initialize(self, options):
        """
        Initialize the ExodusViewer with the supplied filenames.

        Args:
            options: Complete parsed command line parameters from argparse.
        """
        filenames = peacock.utils.getOptionFilenames(options, 'exodus', ['.*\.e', '.*\.e-s[0-9]+'])
        self.onSetFilenames(filenames)

    def onClone(self):
        """
        Clones the current Exodus view.
        """
        filenames = []
        if 'FilePlugin' in self.currentWidget():
            filenames = self.currentWidget()['FilePlugin'].getFilenames()

        super(ExodusViewer, self).onClone()
        self.currentWidget().call('onSetFilenames', filenames)

    def onSetFilenames(self, *args):
        """
        Call the child onSetFilenames.
        """
        for i in range(self.count()):
            self.widget(i).call('onSetFilenames', *args)

    def onInputFileChanged(self, *args):
        """
        Call the child onInputFileChanged methods.
        """
        for i in range(self.count()):
            self.widget(i).call('onInputFileChanged', *args)

    def onStartJob(self, *args):
        """
        Call the child onJobStart methods.
        """
        for i in range(self.count()):
            self.widget(i).call('onJobStart', *args)

    def onCurrentChanged(self, index):
        """
        Updates the menu for the current widget.
        """
        if self._exodus_menu is not None:
            self._exodus_menu.clear()
            for plugin in self.currentWidget()._all_plugins:
                plugin.addToMenu(self._exodus_menu)

    def addToMainMenu(self, menubar):
        """
        Adds the "Result" menu.
        """
        self._exodus_menu = menubar.addMenu("&Results")
        self.onCurrentChanged(0)


def main(size=None):
    """
    A helper function for keeping the tests up-to-date with executing this widget directly from the command-line.
    """
    from PyQt5 import QtWidgets

    plugins = [lambda: VTKWindowPlugin(size=size), FilePlugin, BlockPlugin, MediaControlPlugin, GoldDiffPlugin,
               ColorbarPlugin, MeshPlugin, ClipPlugin, ContourPlugin, CameraPlugin, BackgroundPlugin, OutputPlugin]
    widget = ExodusViewer(plugins=plugins)
    main_window = QtWidgets.QMainWindow()
    main_window.setCentralWidget(widget)
    menubar = main_window.menuBar()
    menubar.setNativeMenuBar(False)
    widget.addToMainMenu(menubar)
    main_window.show()
    return widget, main_window

if __name__ == '__main__':
    import sys
    from PyQt5 import QtWidgets
    from peacock.utils import Testing
    #filenames = Testing.get_chigger_input_list('mesh_only.e', 'mug_blocks_out.e', 'vector_out.e', 'displace.e')
    filenames = Testing.get_chigger_input_list('diffusion_1.e', 'diffusion_2.e')
    app = QtWidgets.QApplication(sys.argv)
    widget, main_window = main()
    widget.onSetFilenames(filenames)
    sys.exit(app.exec_())
