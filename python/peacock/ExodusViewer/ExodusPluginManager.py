import re
from PyQt5 import QtWidgets
import peacock
import mooseutils
from plugins.ExodusPlugin import ExodusPlugin

class ExodusPluginManager(QtWidgets.QWidget, peacock.base.PluginManager):

    def __init__(self, plugins=[]):
        super(ExodusPluginManager, self).__init__(plugins=plugins, plugin_base=ExodusPlugin)

        # The layouts for this widget
        self.setSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Minimum)
        self.MainLayout = QtWidgets.QHBoxLayout()
        self.LeftLayout = QtWidgets.QVBoxLayout()
        self.RightLayout = QtWidgets.QVBoxLayout()
        self.WindowLayout = QtWidgets.QHBoxLayout()

        self.setLayout(self.MainLayout)
        self.MainLayout.addLayout(self.LeftLayout)
        self.MainLayout.addLayout(self.RightLayout)
        self.RightLayout.addLayout(self.WindowLayout)

        self.setup()
        self.LeftLayout.addStretch(1)

        # Set the width of the left-side widgets to that the VTK window gets the space
        width = 0
        for child in self._plugins.itervalues():
            if child.mainLayoutName() == 'LeftLayout':
                width = max(child.sizeHint().width(), width)
        for child in self._plugins.itervalues():
            if child.mainLayoutName() == 'LeftLayout':
                child.setFixedWidth(width)

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
    filenames = ['../../tests/chigger/input/mug_blocks_out.e', '../../tests/chigger/input/displace.e', '../../tests/chigger/input/vector_out.e']
    widget = main()
    widget.initialize(filenames)
    sys.exit(app.exec_())
