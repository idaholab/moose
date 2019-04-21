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

import peacock
import mooseutils

from .PostprocessorPluginManager import PostprocessorPluginManager
from .PostprocessorDataWidget import PostprocessorDataWidget

from .plugins.FigurePlugin import FigurePlugin
from .plugins.PostprocessorSelectPlugin import PostprocessorSelectPlugin
from .plugins.AxesSettingsPlugin import AxesSettingsPlugin
from .plugins.AxisTabsPlugin import AxisTabsPlugin
from .plugins.OutputPlugin import OutputPlugin


class PostprocessorViewer(peacock.base.ViewerBase):
    """
    The main Postprocessor viewer window.

    Usage:
        (1) Create an instance of this widget, passing the type of reader as the only argument. The
            reader should a VectorPostprocessorReader or PostprocessorReader class (not instance).

            widget = PostprocessorViewer()

        (2) Initialize it by passing a list of filenames to the initialize method.

            widget.initialize(filenames)

    Args:
        reader[class]: The reader class to use allow this class to work as a base for VectorPostprocessorViewer.
        timeout[int]: The reload timer duration (in milliseconds)
        plugins[list]: A list of plugin classes to load.
    """

    @staticmethod
    def commandLineArgs(parser):
        parser.add_argument('--postprocessors', '-p', nargs='*', default=[], help="A list of CSV files to open with PostprocessorViewer tab.")


    def __init__(self, reader=mooseutils.PostprocessorReader, timeout=1000,
                       plugins=[FigurePlugin, PostprocessorSelectPlugin, AxesSettingsPlugin, AxisTabsPlugin, OutputPlugin]):

        # Members for this Viewer
        self._timeout = timeout       # duration of data reload timeout, in ms (see PostprocessorDataWidget)
        self._reader_type = reader    # the reader class (not instance)
        self._data = []               # storage for the created PostprocessorDataWidgets
        self._run_start_time = None

        # Call the base constructor
        super(PostprocessorViewer, self).__init__(manager=PostprocessorPluginManager, plugins=plugins)

    def onStartJob(self, csv, path, t):
        """
        Update the 'run' time to avoid loading old data.
        """
        self._run_start_time = t

    def initialize(self, options):
        """
        Initialize the widget by supplying filenames to load.

        Args:
            filenames[list]: A list of filenames to load.
        """
        filenames = peacock.utils.getOptionFilenames(options, 'postprocessors', '.*\.csv')
        self.onSetFilenames(filenames)

    def onClone(self):
        """
        Clones the current Postprocessor view.
        """
        super(PostprocessorViewer, self).onClone()
        self.currentWidget().call('onSetData', self._data)

    def onSetFilenames(self, filenames):
        """
        Call the child onSetFilenames.
        """
        self.setEnabled(True)
        self._data = []
        for fname in filenames:
            reader = self._reader_type(fname, run_start_time=self._run_start_time)
            self._data.append(PostprocessorDataWidget(reader, timer=1000))

        for i in range(self.count()):
            self.widget(i).call('onSetData', self._data)

def main():
    """
    Create Postprocessor and VectorPostprocessors tabs.
    """

    widget = PostprocessorViewer(timeout=None)
    widget.show()
    return widget

if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    filenames = ['../../tests/input/white_elephant_jan_2016.csv']
    widget = main()
    widget.onSetFilenames(filenames)
    sys.exit(app.exec_())
