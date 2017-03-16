import sys, os
from PyQt5 import QtWidgets

import peacock
import mooseutils

from PostprocessorPluginManager import PostprocessorPluginManager
from PostprocessorDataWidget import PostprocessorDataWidget

from plugins.FigurePlugin import FigurePlugin
from plugins.PostprocessorSelectPlugin import PostprocessorSelectPlugin
from plugins.AxesSettingsPlugin import AxesSettingsPlugin
from plugins.AxisTabsPlugin import AxisTabsPlugin
from plugins.OutputPlugin import OutputPlugin


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

    def initialize(self, filenames=[], **kwargs):
        """
        Initialize the widget by supplying filenames to load.

        Args:
            filenames[list]: A list of filenames to load.
        """

        options = kwargs.pop('cmd_line_options', None)
        if options:
            filenames += options.postprocessors
            for arg in options.arguments:
                if arg.endswith(".csv"):
                    filenames.append(os.path.abspath(arg))
            options.postprocessors = filenames # so that we can switch to this tab automatically

        if len(filenames) == 0:
            return

        self.setEnabled(True)
        self._data = [[]] # use list of lists so the ViewerBase.initialize method works with *self._data is passed in to the widget onClone method.
        for fname in filenames:
            reader = self._reader_type(fname, run_start_time=self._run_start_time)
            self._data[0].append(PostprocessorDataWidget(reader, timer=1000))

        for i in range(self.count()):
            self.widget(i).initialize(self._data[0])

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
    widget.initialize(filenames)
    sys.exit(app.exec_())
