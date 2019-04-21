#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import re
from PyQt5 import QtWidgets
import peacock
import mooseutils
from .plugins.ExodusPlugin import ExodusPlugin

class ExodusPluginManager(QtWidgets.QWidget, peacock.base.PluginManager):

    def __init__(self, plugins=[]):
        super(ExodusPluginManager, self).__init__(plugins=plugins, plugin_base=ExodusPlugin)

        self.setSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Preferred)
        self.MainLayout = QtWidgets.QHBoxLayout(self)
        self.LeftLayout = QtWidgets.QVBoxLayout()
        self.RightLayout = QtWidgets.QVBoxLayout()
        self.MainLayout.addLayout(self.LeftLayout)
        self.MainLayout.addLayout(self.RightLayout)
        self.setup()
        self.LeftLayout.addStretch(1)

        # Set the width of the left-side widgets to that the VTK window gets the space
        self.fixLayoutWidth('LeftLayout')

    def repr(self):
        """
        Build the python script.
        """

        # Compile output from the plugins
        output = dict()
        for plugin in self._plugins.itervalues():
            for key, value in plugin.repr().iteritems():
                if key in output:
                    output[key] += value
                else:
                    output[key] = value

        # Add colorbar to window
        if 'colorbar' in output:
            output['window'][0] = 'window = chigger.RenderWindow(result, cbar)'

        # Make import unique
        mooseutils.unique_list(output['imports'], output['imports'])

        # Apply the filters, if they exist
        if 'filters' in output:
            filters = []
            for match in re.findall(r'^(\w+)\s*=', '\n'.join(output['filters']), flags=re.MULTILINE):
                filters.append(match)
            output['result'] += ['result.setOptions(filters=[{}])'.format(', '.join(filters))]

        # Build the script
        string = ''
        for key in ['imports', 'camera', 'reader', 'filters', 'result', 'colorbar', 'window']:
            if key in output:
                string += '\n{}\n'.format('\n'.join(output.pop(key)))

        # Error if keys exist, this means data is missing from the script
        if output:
            raise mooseutils.MooseException('The output data was not completely written, the following keys remain: {}'.format(str(output.keys())))

        return string

    def addToMainMenu(self, menubar):
        exodus_menu = menubar.addMenu("&Results")
        for plugin in self._all_plugins:
            plugin.addToMenu(exodus_menu)

def main(size=None):
    """
    Run window widget alone
    """
    from peacock.ExodusViewer.ExodusPluginManager import ExodusPluginManager
    from plugins.VTKWindowPlugin import VTKWindowPlugin
    from plugins.FilePlugin import FilePlugin
    from plugins.BlockPlugin import BlockPlugin
    from plugins.GoldDiffPlugin import GoldDiffPlugin
    from plugins.ColorbarPlugin import ColorbarPlugin
    from plugins.MeshPlugin import MeshPlugin
    from plugins.BackgroundPlugin import BackgroundPlugin
    from plugins.ClipPlugin import ClipPlugin
    from plugins.ContourPlugin import ContourPlugin
    from plugins.OutputPlugin import OutputPlugin
    from plugins.CameraPlugin import CameraPlugin
    from plugins.MediaControlPlugin import MediaControlPlugin

    plugins = [lambda: VTKWindowPlugin(size=size),
               FilePlugin,
               BlockPlugin,
               MediaControlPlugin,
               GoldDiffPlugin,
               ColorbarPlugin,
               MeshPlugin,
               ClipPlugin,
               ContourPlugin,
               CameraPlugin,
               BackgroundPlugin,
               OutputPlugin]

    widget = ExodusPluginManager(plugins=plugins)
    main_window = QtWidgets.QMainWindow()
    main_window.setCentralWidget(widget)
    menubar = main_window.menuBar()
    menubar.setNativeMenuBar(False)
    widget.addToMainMenu(menubar)
    main_window.show()

    return widget, main_window

if __name__ == '__main__':
    import sys
    app = QtWidgets.QApplication(sys.argv)
    from peacock.utils import Testing
    filenames = Testing.get_chigger_input_list('mug_blocks_out.e', 'displace.e', 'vector_out.e', 'mesh_only.e')
    widget, main_window = main()
    widget.FilePlugin.onSetFilenames(filenames)
    sys.exit(app.exec_())
