#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
from PyQt5 import QtCore, QtWidgets
from peacock.ExodusViewer.plugins.CameraPlugin import CameraPlugin

class MeshCameraPlugin(CameraPlugin):
    """
    Adds a Reload Mesh button to the CameraPlugin.
    """
    reloadMesh = QtCore.pyqtSignal()

    def __init__(self, **kwargs):
        self.ReloadButton = QtWidgets.QPushButton('Reload Mesh')
        super(MeshCameraPlugin, self).__init__(**kwargs)
        self.MainLayout.addWidget(self.ReloadButton)

    def _setupReloadButton(self, qobject):
        qobject.clicked.connect(self._callbackReloadButton)

    def _callbackReloadButton(self):
        self.reloadMesh.emit()
