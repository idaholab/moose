#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys
from PyQt5 import QtWidgets

import mooseutils
import peacock
from .PostprocessorViewer import PostprocessorViewer
from .plugins.FigurePlugin import FigurePlugin
from .plugins.MediaControlPlugin import MediaControlPlugin
from .plugins.PostprocessorSelectPlugin import PostprocessorSelectPlugin
from .plugins.AxesSettingsPlugin import AxesSettingsPlugin
from .plugins.AxisTabsPlugin import AxisTabsPlugin
from .plugins.OutputPlugin import OutputPlugin


class VectorPostprocessorViewer(PostprocessorViewer):
    """
    The main VectorPostprocessor viewer window.

    Usage:
        (1) Create an instance of this widget, passing the type of reader as the only argument. The
            reader should a VectorPostprocessorReader or PostprocessorReader class (not instance).

            widget = VectorPostprocessorViewer()

        (2) Initialize it by passing a list of filenames to the initialize method.

            widget.initialize(filenames)

    Args:
        reader[class]: The reader class to use allow this class to work as a base for VectorPostprocessorViewer.
        timeout[int]: The reload timer duration (in milliseconds)
        plugins[list]: A list of plugin classes to load.
    """

    @staticmethod
    def commandLineArgs(parser):
        parser.add_argument('--vectorpostprocessors', '-v', nargs='*', default=[], help="A list of CSV file patterns to open with VectorPostprocessorViewer tab.")

    def __init__(self, plugins=[FigurePlugin, MediaControlPlugin, PostprocessorSelectPlugin, AxesSettingsPlugin, AxisTabsPlugin, OutputPlugin], **kwargs):
        super(VectorPostprocessorViewer, self).__init__(reader=mooseutils.VectorPostprocessorReader, plugins=plugins, **kwargs)

    def initialize(self, options):
        """
        Initialize the manager by appending supplied files from parser.
        """
        filenames = peacock.utils.getOptionFilenames(options, 'vectorpostprocessors', '.*\*\.csv')
        self.onSetFilenames(filenames)

def main():
    """
    Create VectorPostprocessors tabs.
    """
    widget = VectorPostprocessorViewer(timeout=None)
    widget.show()
    return widget

if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    filenames = ['../../tests/input/vpp2_*.csv']
    widget = main()
    widget.initialize(filenames)
    sys.exit(app.exec_())
