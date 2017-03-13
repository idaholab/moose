import sys
import peacock
from PyQt5 import QtWidgets
from ExodusPlugin import ExodusPlugin

class OutputPlugin(peacock.base.OutputWidgetBase, ExodusPlugin):
    """
    Plugin responsible for triggering the creation of png/py files and live script viewing.
    """

    def __init__(self):
        super(OutputPlugin, self).__init__()
        self.MainLayout.addStretch()
        self.setup()

    def repr(self):
        """
        Return matplotlib scripting information.
        """
        window_options, window_sub_options = self._window.options().toScriptString()

        output = dict()
        output['window'] = ['window = chigger.RenderWindow(result)']
        output['window'] += ['window.setOptions({})'.format(', '.join(window_options))]
        for key, value in window_sub_options.iteritems():
            output['window'] += ['window.setOptions({}, {})'.format(repr(key), ', '.join(value))]

        output['window'] += ['window.start()']
        return output

    def onCameraChanged(self, *args):
        """
        Slot that is called when the camera is changed.
        """
        self.updateLiveScriptText()

    def onWindowUpdated(self):
        """
        Slot called when the window is changed.
        """
        self.updateLiveScriptText()

    def _setupPDFButton(self, qobject):
        """
        Remove the PDF button.
        """
        qobject.setVisible(False)

def main(size=None):
    """
    Run the ContourPlugin all by its lonesome.
    """
    from peacock.ExodusViewer.ExodusPluginManager import ExodusPluginManager
    from peacock.ExodusViewer.plugins.VTKWindowPlugin import VTKWindowPlugin
    widget = ExodusPluginManager(plugins=[lambda: VTKWindowPlugin(size=size), OutputPlugin])
    widget.show()

    return widget, widget.VTKWindowPlugin

if __name__ == '__main__':
    from peacock.utils import Testing
    app = QtWidgets.QApplication(sys.argv)
    filename = Testing.get_chigger_input('mug_blocks_out.e')
    widget, window = main()
    window.initialize([filename])
    sys.exit(app.exec_())
