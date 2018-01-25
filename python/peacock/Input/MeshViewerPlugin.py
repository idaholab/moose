#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

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
        self._use_test_objects = True

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
            self.reset()

        self.meshEnabled.emit(False)
        if not tree.app_info.valid():
            return
        # if we aren't writing out the mesh node then don't show it
        mesh_node = tree.getBlockInfo("/Mesh")
        if not mesh_node or not mesh_node.included:
            self.onFileChanged()
            self.setLoadingMessage("Mesh block not included")
            return
        exe_path = tree.app_info.path
        self._removeFileNoError(self.current_temp_mesh_file)
        self.current_temp_mesh_file = os.path.abspath(self.temp_mesh_file)
        input_file = os.path.abspath(self.temp_input_file)
        self._removeFileNoError(self.current_temp_mesh_file)
        self._removeFileNoError(input_file)
        self.needInputFile.emit(input_file)

        if not os.path.exists(input_file):
            self.meshEnabled.emit(False)
            self.onFileChanged()
            self.setLoadingMessage("Error reading temporary input file")
            return

        try:
            args = ["-i", input_file, "--mesh-only", self.current_temp_mesh_file]
            if self._use_test_objects:
                args.append("--allow-test-objects")
            ExeLauncher.runExe(exe_path, args, print_errors=False)
            self.meshEnabled.emit(True)
            self.onFileChanged(self.current_temp_mesh_file)
        except Exception:
            self.meshEnabled.emit(False)
            self.onFileChanged()
            self.setLoadingMessage("Error producing mesh")
            self._removeFileNoError(self.current_temp_mesh_file)

        self._removeFileNoError(input_file) # we need the mesh file since it is in use but not the input file

    def closing(self):
        self._removeFileNoError(self.temp_input_file)
        self._removeFileNoError(self.current_temp_mesh_file)
