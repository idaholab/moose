from peacock.ExodusViewer.plugins.VTKWindowPlugin import VTKWindowPlugin
from peacock.utils import ExeLauncher
from PyQt5.QtCore import pyqtSignal
import os

class MeshViewerPlugin(VTKWindowPlugin):
    """
    A VTKWindowPlugin that shows the mesh of the input file as given
    by --mesh-only
    Signals:
        needInputFile[str]: Emitted when an input file needs to be written out
        meshEnabled[bool]: Notifies when the mesh has been enabled/disabled
    """
    needInputFile = pyqtSignal(str)
    meshEnabled = pyqtSignal(bool)

    def __init__(self, **kwargs):
        super(MeshViewerPlugin, self).__init__(**kwargs)
        self.temp_input_file = "peacock_run_mesh_tmp.i"
        self.temp_mesh_file = "peacock_run_mesh_tmp.e"
        self.current_temp_mesh_file = os.path.abspath(self.temp_mesh_file)
        self.hide()

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

    def meshChanged(self, tree):
        """
        The parameters of the mesh has changed.
        We need to update the view of the mesh by generating a new mesh file.
        """
        self.meshEnabled.emit(False)
        self.hide()
        if not tree.app_info.valid():
            return
        # if we aren't writing out the mesh node then don't show it
        mesh_node = tree.getBlockInfo("/Mesh")
        if not mesh_node or not mesh_node.included:
            return
        exe_path = tree.app_info.path
        self._removeFileNoError(self.current_temp_mesh_file)
        self.current_temp_mesh_file = os.path.abspath(self.temp_mesh_file)
        input_file = os.path.abspath(self.temp_input_file)
        self._removeFileNoError(self.current_temp_mesh_file)
        self._removeFileNoError(input_file)
        self.needInputFile.emit(input_file)
        try:
            args = ["-i", input_file, "--mesh-only", self.current_temp_mesh_file]
            ExeLauncher.runExe(exe_path, args, print_errors=False)
            self.show()
            self.meshEnabled.emit(True)
            self.onFileChanged(self.current_temp_mesh_file)
        except Exception:
            self.hide()
            self.meshEnabled.emit(False)
            self._removeFileNoError(self.current_temp_mesh_file)


        self._removeFileNoError(input_file) # we need the mesh file since it is in use but not the input file

    def onBlockChanged(self, block, tree):
        if block.path == "/Mesh" or block.path.startswith("/Mesh/"):
            self.meshChanged(tree)

    def closing(self):
        self._removeFileNoError(self.temp_input_file)
        self._removeFileNoError(self.current_temp_mesh_file)

