#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import re
from PyQt5 import QtWidgets, QtCore
import peacock
import mooseutils
from plugins.ExodusPlugin import ExodusPlugin

class ExodusPluginManager(QtWidgets.QWidget, peacock.base.PluginManager):

    def __init__(self, plugins=[]):
        super(ExodusPluginManager, self).__init__(plugins=plugins, plugin_base=ExodusPlugin)

        # The layouts for this widget
        self.setSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Preferred)
        self.MainLayout = QtWidgets.QHBoxLayout()

        self.LeftScrollArea = QtWidgets.QScrollArea()
        self.LeftScrollArea.setWidgetResizable(True)
        self.LeftScrollArea.setFrameShadow(QtWidgets.QFrame.Plain)
        self.LeftScrollArea.setStyleSheet("QScrollArea { background: transparent; }");
        self.LeftScrollArea.viewport().setStyleSheet(".QWidget { background: transparent; }");
        self.LeftScrollArea.setFrameShape(QtWidgets.QFrame.NoFrame)
        self.LeftScrollArea.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.LeftScrollContent = QtWidgets.QWidget()
        self.LeftScrollArea.setWidget(self.LeftScrollContent)
        self.LeftLayout = QtWidgets.QVBoxLayout()
        self.LeftScrollContent.setLayout(self.LeftLayout)
        self.LeftLayout.setContentsMargins(0, 0, 0, 0)
        self.RightLayout = QtWidgets.QVBoxLayout()
        self.WindowLayout = QtWidgets.QHBoxLayout()

        self.setLayout(self.MainLayout)
        self.MainLayout.addWidget(self.LeftScrollArea)
        self.MainLayout.addLayout(self.RightLayout)
        self.RightLayout.addLayout(self.WindowLayout)
        self.setup()
        self.LeftLayout.addStretch(1)

        # Set the width of the left-side widgets to that the VTK window gets the space
        self.fixLayoutWidth('LeftLayout')
        self.LeftScrollContent.setFixedWidth(self.LeftLayout.sizeHint().width())
        self.LeftScrollArea.setFixedWidth(self.LeftScrollContent.width() + 15) # This gets rid of the horizontal "wiggle"

        if 'BlockPlugin' in self:
            self['BlockPlugin'].setCollapsed(True)
        if 'BackgroundPlugin' in self:
            self['BackgroundPlugin'].setCollapsed(True)
        if 'CameraPlugin' in self:
            self['CameraPlugin'].setCollapsed(True)

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
        for key in ['imports', 'camera', 'reader', 'filters', 'result', 'window']:
            if key in output:
                string += '\n{}\n'.format('\n'.join(output.pop(key)))

        # Error if keys exist, this means data is missing from the script
        if output:
            raise mooseutils.MooseException('The output data was not completely written, the following keys remain: {}'.format(str(output.keys())))

        return string


def main(size=None):
    """
    Run window widget alone
    """
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

    widget = ExodusPluginManager(plugins=[lambda: VTKWindowPlugin(size=size), BlockPlugin, MediaControlPlugin, VariablePlugin, FilePlugin, GoldDiffPlugin,  MeshPlugin, BackgroundPlugin, ClipPlugin, ContourPlugin, CameraPlugin, OutputPlugin])

    widget.show()

    return widget

if __name__ == '__main__':
    import sys
    app = QtWidgets.QApplication(sys.argv)
    from peacock.utils import Testing
    filenames = Testing.get_chigger_input_list('mesh_only.e', 'mug_blocks_out.e', 'vector_out.e', 'displace.e')
    widget = main()
    widget.FilePlugin.onSetFilenames(filenames)
    sys.exit(app.exec_())
