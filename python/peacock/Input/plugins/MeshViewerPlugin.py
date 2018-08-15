#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import chigger
from peacock.ExodusViewer.plugins.VTKWindowPlugin import VTKWindowPlugin
from peacock.utils import ExeLauncher
from PyQt5 import QtCore

import mooseutils

class MeshViewerPlugin(VTKWindowPlugin):
    """
    A VTKWindowPlugin that shows the mesh of the input file as given
    by --mesh-only
    Signals:
        needInputFile[str]: Emitted when an input file needs to be written out
        meshEnabled[bool]: Notifies when the mesh has been enabled/disabled
    """
    needInputFile = QtCore.pyqtSignal(str)
    meshEnabled = QtCore.pyqtSignal(bool)

    @staticmethod
    def getDefaultResultOptions():
        """
        Return the default options for the ExodusResult object.
        """
        return dict(representation='wireframe')

    def __init__(self, **kwargs):
        super(MeshViewerPlugin, self).__init__(**kwargs)
        self.temp_input_file = "peacock_run_mesh_tmp.i"
        self.temp_mesh_file = "peacock_run_mesh_tmp_{}.e"
        self.current_temp_mesh_file = os.path.abspath(self.temp_mesh_file)
        self._use_test_objects = True
        self.setMainLayoutName('WindowLayout')

    def _removeFileNoError(self, fname):
        """
        Utility function to remove a file while ignoring errors.
        Input:
            fname[str]: path to remove
        """
        try:
            path = os.path.abspath(fname)
            os.remove(path)
        except:
            pass

    def onHighlight(self, block=None, boundary=None, nodeset=None):
        """
        Highlight the desired block/boundary/nodeset.

        To remove highlight call this function without the inputs set.

        Args:
            block[list]: List of block ids to highlight.
            boundary[list]: List of boundary ids to highlight.
            nodeset[list]: List of nodeset ids to highlight.
        """
        if not self._initialized:
            return

        if not self._highlight:
            self._highlight = chigger.exodus.ExodusResult(self._reader,
                                                          renderer=self._result.getVTKRenderer(),
                                                          color=[1,0,0])

        if (block or boundary or nodeset):
            self._highlight.setOptions(block=block, boundary=boundary, nodeset=nodeset)
            self._highlight.setOptions(edges=True, edge_width=3, edge_color=[1,0,0], point_size=5)
            self._window.append(self._highlight)
        else:
            self._highlight.reset()
            self._window.remove(self._highlight)
            self._highlight = None

        self.onWindowRequiresUpdate()

    def useTestObjects(self, use_test_objs):
        """
        Set to pass the --allow-test-objects flag while generating the mesh
        """
        self._use_test_objects = use_test_objs

    def meshChanged(self, tree, reset=False):
        """
        The parameters of the mesh has changed.
        We need to update the view of the mesh by generating a new mesh file.
        """
        if reset:
            self._reset()

        self.meshEnabled.emit(False)
        if not tree.app_info.valid():
            return
        # if we aren't writing out the mesh node then don't show it
        mesh_node = tree.getBlockInfo("/Mesh")
        if not mesh_node or not mesh_node.included:
            self.onSetFilename(None)
            self.onWindowRequiresUpdate()
            self._setLoadingMessage("Mesh block not included")
            return

        exe_path = tree.app_info.path
        self._removeFileNoError(self.current_temp_mesh_file)
        input_filename = ""
        if tree.input_filename:
            input_filename = os.path.basename(os.path.splitext(tree.input_filename)[0])
        self.current_temp_mesh_file = os.path.abspath(self.temp_mesh_file.format(input_filename))
        input_file = os.path.abspath(self.temp_input_file)
        self._removeFileNoError(self.current_temp_mesh_file)
        self._removeFileNoError(input_file)
        self.needInputFile.emit(input_file)

        if not os.path.exists(input_file):
            self.meshEnabled.emit(False)
            self.onSetFilename(None)
            self.onWindowRequiresUpdate()
            self._setLoadingMessage("Error reading temporary input file")
            return
        try:
            args = ["-i", input_file, "--mesh-only", self.current_temp_mesh_file]
            if self._use_test_objects:
                args.append("--allow-test-objects")
            ExeLauncher.runExe(exe_path, args, print_errors=False)
            self.meshEnabled.emit(True)
            self.onSetFilename(self.current_temp_mesh_file)
            self.onWindowRequiresUpdate()

        except Exception as e:
            self.meshEnabled.emit(False)
            self.onSetFilename(None)
            self.onWindowRequiresUpdate()
            mooseutils.mooseWarning("Error producing mesh: %s" % e)
            self._setLoadingMessage("Error producing mesh")
            self._removeFileNoError(self.current_temp_mesh_file)

        self._removeFileNoError(input_file) # we need the mesh file since it is in use but not the input file

    def closing(self):
        self._removeFileNoError(self.temp_input_file)
        self._removeFileNoError(self.current_temp_mesh_file)

def main(size=None):
    """
    Run the VTKWindowPlugin all by its lonesome.
    """
    from peacock.ExodusViewer.ExodusPluginManager import ExodusPluginManager
    plugin = MeshViewerPlugin(size=size)
    plugin.setMainLayoutName('RightLayout')
    widget = ExodusPluginManager(plugins=[lambda: plugin])
    widget.show()
    return widget, widget.MeshViewerPlugin

if __name__ == "__main__":
    import sys
    from peacock.utils import Testing
    from PyQt5 import QtWidgets
    app = QtWidgets.QApplication(sys.argv)
    filename = Testing.get_chigger_input('mesh_only.e')
    widget, window = main(size=[600,600])
    window.onSetFilename(filename)
    window.onSetVariable('diffused')
    window.onWindowRequiresUpdate()
    sys.exit(app.exec_())
