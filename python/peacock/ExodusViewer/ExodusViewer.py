#!/usr/bin/env python
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import peacock
from ExodusPluginManager import ExodusPluginManager
from plugins.VTKWindowPlugin import VTKWindowPlugin
from plugins.FilePlugin import FilePlugin
from plugins.GoldDiffPlugin import GoldDiffPlugin
from plugins.VariablePlugin import VariablePlugin
from plugins.MeshPlugin import MeshPlugin
from plugins.BackgroundPlugin import BackgroundPlugin
from plugins.ClipPlugin import ClipPlugin
from plugins.ContourPlugin import ContourPlugin
from plugins.OutputPlugin import OutputPlugin
from plugins.CameraPlugin import CameraPlugin
from plugins.MediaControlPlugin import MediaControlPlugin
from plugins.BlockPlugin import BlockPlugin

class ExodusViewer(peacock.base.ViewerBase):
    """
    Widget for viewing ExodusII results.
    """

    @staticmethod
    def commandLineArgs(parser):
        parser.add_argument('--exodus', '-r', nargs='*', default=[], help="A list of ExodusII files to open.")

    def __init__(self, plugins=[FilePlugin, GoldDiffPlugin, VariablePlugin, \
                                MeshPlugin, BackgroundPlugin, ClipPlugin, ContourPlugin, CameraPlugin, \
                                OutputPlugin, VTKWindowPlugin, MediaControlPlugin,
                                lambda: BlockPlugin(layout='RightLayout')]):
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


def main(size=None):
    """
    A helper function for keeping the tests up-to-date with executing this widget directly from the command-line.
    """
    plugins=[lambda: VTKWindowPlugin(size=size), FilePlugin, GoldDiffPlugin, VariablePlugin, \
             MeshPlugin, BackgroundPlugin, ClipPlugin, ContourPlugin, CameraPlugin, \
             OutputPlugin, BlockPlugin, MediaControlPlugin]
    widget = ExodusViewer(plugins=plugins)
    widget.show()
    return widget

if __name__ == '__main__':
    import sys
    from PyQt5 import QtWidgets
    from peacock.utils import Testing
    filenames = Testing.get_chigger_input_list('mesh_only.e', 'mug_blocks_out.e', 'vector_out.e', 'displace.e')
    app = QtWidgets.QApplication(sys.argv)
    widget = main()
    widget.onSetFilenames(filenames)
    sys.exit(app.exec_())
