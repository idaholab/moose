from PyQt5 import QtCore, QtWidgets
import peacock
import mooseutils

class PostprocessorPluginManager(QtWidgets.QWidget, peacock.base.PluginManager):
    """
    Widget containing all of the Postprocessor plugins.

    This widget contains two layouts for the plugin placements: "LeftLayout" and "RightLayout"
    """

    def __init__(self, plugins=[]):
        super(PostprocessorPluginManager, self).__init__(plugins=plugins)

        # Member variables
        self._data = None

        # The layouts for this widget
        self.MainLayout = QtWidgets.QHBoxLayout()
        self.setLayout(self.MainLayout)
        self.LeftLayout = QtWidgets.QVBoxLayout()
        self.RightLayout = QtWidgets.QVBoxLayout()

        self.MainLayout.addLayout(self.LeftLayout)
        self.MainLayout.addLayout(self.RightLayout)

        self.setup()
        self.LeftLayout.addStretch(1)

    def initialize(self, data):
        """
        Set the data for this widget to operate on.

        Args:
            data[list]: A list of PostprocessorDataWidget object to be used in this widget.
        """

        # Update the local storage
        self._data = data

        # Call the plugin initialize methods
        super(PostprocessorPluginManager, self).initialize(self._data)

        # Fix the width of left-side plugins to the maximum of all the others.
        width = 0
        for plugin in self._plugins.itervalues():
            if plugin.mainLayoutName() == 'LeftLayout':
                width = max(width, plugin.minimumSizeHint().width())
        for plugin in self._plugins.itervalues():
            if plugin.mainLayoutName() == 'LeftLayout':
                plugin.setFixedWidth(width)

        self.adjustSize()

    def repr(self):
        """
        Writes a python script for reproducing the actions of this widget.
        """

        # Setup
        output = []
        imports = []

        # Build script from plugins
        for plugin in self._plugins.itervalues():
            out, imp = plugin.repr()
            output += out
            mooseutils.unique_list(imports, imp)

        return '\n'.join(imports + [''] + output)

def main():
    """
    Run plot widget alone
    """
    from plugins.FigurePlugin import FigurePlugin
    from plugins.MediaControlPlugin import MediaControlPlugin
    from plugins.PostprocessorSelectPlugin import PostprocessorSelectPlugin
    from plugins.AxesSettingsPlugin import AxesSettingsPlugin
    from plugins.AxisTabsPlugin import AxisTabsPlugin
    from plugins.OutputPlugin import OutputPlugin

    widget = PostprocessorPluginManager(plugins=[FigurePlugin, MediaControlPlugin, PostprocessorSelectPlugin, AxesSettingsPlugin, AxisTabsPlugin, OutputPlugin])
    widget.FigurePlugin.setFixedSize(QtCore.QSize(600,600))
    widget.show()

    return widget, widget.FigurePlugin

if __name__ == '__main__':
    import sys
    from PostprocessorDataWidget import PostprocessorDataWidget

    app = QtWidgets.QApplication(sys.argv)

    data = [PostprocessorDataWidget(mooseutils.PostprocessorReader('../../tests/input/white_elephant_jan_2016.csv'))]
    #data = [PostprocessorDataWidget(mooseutils.VectorPostprocessorReader('../../tests/input/vpp_*.csv')),
    #        PostprocessorDataWidget(mooseutils.VectorPostprocessorReader('../../tests/input/vpp2_*.csv'))]
    widget, _ = main()
    widget.initialize(data)
    sys.exit(app.exec_())
