#!/usr/bin/env python
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
import os

class ExodusViewer(peacock.base.ViewerBase):
    """
    Widget for viewing ExodusII results.
    """

    @staticmethod
    def commandLineArgs(parser):
        parser.add_argument('--exodus', '-r', nargs='*', default=[], help="A list of ExodusII files to open.")

    def __init__(self, plugins=[VTKWindowPlugin, FilePlugin, GoldDiffPlugin, VariablePlugin, \
                                MeshPlugin, BackgroundPlugin, ClipPlugin, ContourPlugin, CameraPlugin, \
                                OutputPlugin, MediaControlPlugin, BlockPlugin]):
        super(ExodusViewer, self).__init__(manager=ExodusPluginManager, plugins=plugins)

    def initialize(self, filenames=[], **kwargs):
        """
        Initialize the ExodusViewer with the supplied filenames.

        Args:
            filenames[list]: List of filenames to load.

        Kwargs:
            cmd_line_options: (Optional) Complete parsed command line parameters from argparse.
        """

        options = kwargs.pop('cmd_line_options', None)
        if options:
            filenames += options.exodus
            for arg in options.arguments:
                if arg.endswith(".e"):
                    filenames.append(os.path.abspath(arg))
            options.exodus = filenames # so that we can switch to this tab automatically

        if len(filenames) == 0:
            return

        super(ExodusViewer, self).initialize(filenames)

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
    filenames = ['../../tests/chigger/input/mesh_only.e', '../../tests/chigger/input/mug_blocks_out.e', '../../tests/chigger/input/vector_out.e', '../../tests/chigger/input/displace.e']
    app = QtWidgets.QApplication(sys.argv)
    widget = main()
    widget.initialize(filenames)
    sys.exit(app.exec_())
