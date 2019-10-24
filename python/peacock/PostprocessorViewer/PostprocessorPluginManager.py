#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

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

        # The layouts for this widget
        self.MainLayout = QtWidgets.QHBoxLayout()
        self.setLayout(self.MainLayout)
        self.LeftLayout = QtWidgets.QVBoxLayout()
        self.RightLayout = QtWidgets.QVBoxLayout()

        self.MainLayout.addLayout(self.LeftLayout)
        self.MainLayout.addLayout(self.RightLayout)

        self.setup()

    def repr(self):
        """
        Writes a python script for reproducing the actions of this widget.
        """

        # Setup
        output = []
        imports = []

        # Build script from plugins
        for plugin in self._plugins.values():
            out, imp = plugin.repr()
            output += out
            mooseutils.unique_list(imports, imp)

        return '\n'.join(imports + [''] + output)

def main():
    """
    Run plot widget alone
    """
    from .plugins.FigurePlugin import FigurePlugin
    from .plugins.MediaControlPlugin import MediaControlPlugin
    from .plugins.PostprocessorSelectPlugin import PostprocessorSelectPlugin
    from .plugins.AxesSettingsPlugin import AxesSettingsPlugin
    from .plugins.AxisTabsPlugin import AxisTabsPlugin
    from .plugins.OutputPlugin import OutputPlugin

    widget = PostprocessorPluginManager(plugins=[FigurePlugin, MediaControlPlugin, PostprocessorSelectPlugin, AxesSettingsPlugin, AxisTabsPlugin, OutputPlugin])
    widget.FigurePlugin.setFixedSize(QtCore.QSize(600,600))
    widget.show()

    return widget, widget.FigurePlugin

if __name__ == '__main__':
    import sys
    from .PostprocessorDataWidget import PostprocessorDataWidget

    app = QtWidgets.QApplication(sys.argv)

    data = [PostprocessorDataWidget(mooseutils.PostprocessorReader('../tests/input/white_elephant_jan_2016.csv'))]
    #data = [PostprocessorDataWidget(mooseutils.VectorPostprocessorReader('../tests/input/vpp_*.csv')),
    #        PostprocessorDataWidget(mooseutils.VectorPostprocessorReader('../tests/input/vpp2_*.csv'))]
    widget, _ = main()
    widget.call('onSetData', data)
    sys.exit(app.exec_())
