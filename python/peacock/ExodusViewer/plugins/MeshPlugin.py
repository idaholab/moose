#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys
from PyQt5 import QtCore, QtWidgets
import chigger
from .ExodusPlugin import ExodusPlugin

class MeshPlugin(QtWidgets.QGroupBox, ExodusPlugin):
    """
    Controls for the mesh appearance.
    """

    #: Emitted when window has changed
    windowRequiresUpdate = QtCore.pyqtSignal()

    #: pyqtSignal: Emitted when the chigger objects options are changed
    readerOptionsChanged = QtCore.pyqtSignal(dict)
    resultOptionsChanged = QtCore.pyqtSignal(dict)

    #: pyqtSignal: Add/remove result objects
    appendResult = QtCore.pyqtSignal(chigger.base.ChiggerResultBase)
    removeResult = QtCore.pyqtSignal(chigger.base.ChiggerResultBase)

    #: pyqtSignal: Emitted when adding a filter
    addFilter = QtCore.pyqtSignal(chigger.filters.ChiggerFilterBase)

    #: pyqtSignal: Emitted when removing a filter
    removeFilter = QtCore.pyqtSignal(chigger.filters.ChiggerFilterBase)

    def __init__(self, **kwargs):
        super(MeshPlugin, self).__init__(**kwargs)

        self._preferences.addBool("exodus/viewMesh",
                "View the mesh",
                False,
                "View the mesh by default",
                )

        self._transform = chigger.filters.TransformFilter()
        self._extents = None

        # QGroupBox settings
        self.setSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Minimum)

        # Main Layout
        self.MainLayout = QtWidgets.QHBoxLayout(self)

        # Displacements
        self.DisplacementToggle = QtWidgets.QCheckBox("Displacements")
        self.DisplacementMagnitude = QtWidgets.QDoubleSpinBox()

        # Mesh
        self.RepresentationLabel = QtWidgets.QLabel("Representation:")
        self.Representation = QtWidgets.QComboBox()
        self.ViewMeshToggle = QtWidgets.QCheckBox('View Mesh')

        # Scale
        self.ScaleXLabel = QtWidgets.QLabel("Scale X:")
        self.ScaleYLabel = QtWidgets.QLabel("Scale Y:")
        self.ScaleZLabel = QtWidgets.QLabel("Scale Z:")
        self.ScaleX = QtWidgets.QDoubleSpinBox()
        self.ScaleY = QtWidgets.QDoubleSpinBox()
        self.ScaleZ = QtWidgets.QDoubleSpinBox()

        # Extents
        self.Extents = QtWidgets.QCheckBox("Extents")

        # Layouts
        self.LeftLayout = QtWidgets.QVBoxLayout()
        self.DisplacementLayout = QtWidgets.QHBoxLayout()
        self.DisplacementLayout.addWidget(self.DisplacementToggle)
        self.DisplacementLayout.addSpacing(5)
        self.DisplacementLayout.addWidget(self.DisplacementMagnitude)

        # Toggles
        self.MeshViewLayout = QtWidgets.QHBoxLayout()
        self.MeshViewLayout.addWidget(self.ViewMeshToggle)
        self.MeshViewLayout.addSpacing(15)
        self.MeshViewLayout.addWidget(self.Extents)
        self.MeshViewLayout.addStretch(1)

        # Representation
        self.RepresentationLayout = QtWidgets.QHBoxLayout()
        self.RepresentationLayout.addWidget(self.RepresentationLabel)
        self.RepresentationLayout.addSpacing(5)
        self.RepresentationLayout.addWidget(self.Representation)

        # Left column
        self.LeftLayout.addLayout(self.MeshViewLayout)
        self.LeftLayout.addLayout(self.DisplacementLayout)
        self.LeftLayout.addLayout(self.RepresentationLayout)

        # Right column (scale boxes)
        self.RightLayout = QtWidgets.QGridLayout()
        self.RightLayout.setHorizontalSpacing(5)
        self.RightLayout.addWidget(self.ScaleXLabel, 0, 0)
        self.RightLayout.addWidget(self.ScaleX, 0, 1)
        self.RightLayout.addWidget(self.ScaleYLabel, 1, 0)
        self.RightLayout.addWidget(self.ScaleY, 1, 1)
        self.RightLayout.addWidget(self.ScaleZLabel, 2, 0)
        self.RightLayout.addWidget(self.ScaleZ, 2, 1)

        # Main
        self.MainLayout.addLayout(self.LeftLayout)
        self.MainLayout.addSpacing(15)
        self.MainLayout.addLayout(self.RightLayout)
        self.MainLayout.setSpacing(0)

        self.setup()

    def onSetupResult(self, result):
        """
        Create the filters and load the stored state when the ExodusResult is created.
        """
        self._extents = chigger.misc.VolumeAxes(result)
        if self.Extents.isChecked():
            self._extents.update()
        else:
            self._extents.reset()

    def onResetWindow(self):
        """
        Delete filters when the window is destroyed.
        """
        self._extents.reset()
        self._extents = None

    def _loadPlugin(self):
        """
        Helper for loading plugin state.
        """
        self.load(self.Extents)
        if not self.hasState(self.Extents):
            self.Extents.setChecked(False)

        self.load(self.DisplacementToggle)
        if not self.hasState(self.DisplacementToggle):
            self.DisplacementToggle.setChecked(True)

        self.load(self.DisplacementMagnitude)
        if not self.hasState(self.DisplacementMagnitude):
            self.DisplacementMagnitude.setValue(1.0)

        self.load(self.Representation)
        if not self.hasState(self.Representation):
            self.Representation.setCurrentIndex(0)

        self.load(self.ViewMeshToggle)
        if not self.hasState(self.ViewMeshToggle):
            self.ViewMeshToggle.setChecked(self._preferences.value("exodus/viewMesh"))

        self.load(self.ScaleX)
        if not self.hasState(self.ScaleX):
            self.ScaleX.setValue(1.0)

        self.load(self.ScaleY)
        if not self.hasState(self.ScaleY):
            self.ScaleY.setValue(1.0)

        self.load(self.ScaleZ)
        if not self.hasState(self.ScaleZ):
            self.ScaleZ.setValue(1.0)

    def updateReaderOptions(self):
        """
        Update ExodusReaderOptions
        """
        reader_options = dict()
        reader_options['displacements'] = bool(self.DisplacementToggle.isChecked())
        reader_options['displacement_magnitude'] = self.DisplacementMagnitude.value()
        self.DisplacementMagnitude.setEnabled(reader_options['displacements'])
        self.readerOptionsChanged.emit(reader_options)

    def updateResultOptions(self):
        """
        Update ExodusResult options.
        """
        result_options = dict()
        result_options['representation'] = str(self.Representation.currentText()).lower()
        result_options['edges'] = self.ViewMeshToggle.isChecked()
        result_options['edge_color'] = [0, 0, 0]
        self.resultOptionsChanged.emit(result_options)

    def updateOptions(self):
        """
        Updates the results for the changes to the mesh view.
        """
        scale = [self.ScaleX.value(), self.ScaleY.value(), self.ScaleZ.value()]
        if scale != [1, 1, 1]:
            self._transform.setOption('scale', scale)
            self.addFilter.emit(self._transform)
        else:
            self.removeFilter.emit(self._transform)

        if self._extents is not None:
            if self.Extents.isChecked():
                self._extents.update()
            else:
                self._extents.reset()

        # Emit the update signal with the new arguments
        self.updateReaderOptions()
        self.updateResultOptions()

    def _setupDisplacementToggle(self, qobject):
        """
        Setup method for DisplacementToggle widget. (protected)
        """
        qobject.setChecked(True)
        qobject.stateChanged.connect(self._callbackDisplacementToggle)

    def _callbackDisplacementToggle(self, *args):
        self.store(self.DisplacementToggle)
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _setupDisplacementMagnitude(self, qobject):
        """
        Setup for DisplacementMagnitude widget. (protected)
        """
        qobject.setSingleStep(0.1)
        qobject.setValue(1.0)
        qobject.valueChanged.connect(self._callbackDisplacementMagnitude)

    def _callbackDisplacementMagnitude(self, *args):
        self.store(self.DisplacementMagnitude)
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _setupRepresentation(self, qobject):
        """
        Setup from Representation widget. (protected)
        """
        qobject.addItem('Surface')
        qobject.addItem('Wireframe')
        qobject.addItem('Points')
        qobject.setCurrentIndex(0)
        qobject.currentIndexChanged.connect(self._callbackRepresentation)

    def _callbackRepresentation(self):
        """
        Callback for Representation widget. (protected)
        """
        self.store(self.Representation)
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _setupViewMeshToggle(self, qobject):
        """
        Setup for showing the ViewMeshToggle widget. (protected)
        """
        qobject.setChecked(self._preferences.value("exodus/viewMesh"))
        qobject.stateChanged.connect(lambda value: self._callbackViewMeshToggle())

    def _callbackViewMeshToggle(self):
        """
        Callback for ViewMeshToggle widget. (protected)
        """
        self.store(self.ViewMeshToggle)
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _prefCallbackViewMesh(self, value):
        self.ViewMeshToggle.setChecked(value)
        self.store(self.ViewMeshToggle)
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _setupScaleX(self, qobject):
        """
        Helper method for adding various scale related widgets. (protected)
        """
        qobject.setSingleStep(0.1)
        qobject.setValue(1.0)
        qobject.setMinimum(0.1)
        qobject.valueChanged.connect(self._callbackScaleX)
        qobject.setMaximum(1000)

    def _setupScaleY(self, qobject):
        """
        Helper method for adding various scale related widgets. (protected)
        """
        qobject.setSingleStep(0.1)
        qobject.setValue(1.0)
        qobject.setMinimum(0.1)
        qobject.valueChanged.connect(self._callbackScaleY)
        qobject.setMaximum(1000)

    def _setupScaleZ(self, qobject):
        """
        Helper method for adding various scale related widgets. (protected)
        """
        qobject.setSingleStep(0.1)
        qobject.setValue(1.0)
        qobject.setMinimum(0.1)
        qobject.valueChanged.connect(self._callbackScaleZ)
        qobject.setMaximum(1000)

    def _callbackScaleX(self):
        self.store(self.ScaleX)
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _callbackScaleY(self):
        self.store(self.ScaleY)
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _callbackScaleZ(self):
        self.store(self.ScaleZ)
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _setupExtents(self, qobject):
        """
        Setup method for the extents toggle.
        """
        qobject.stateChanged.connect(self._callbackExtents)

    def _callbackExtents(self, value):
        """
        Enables/disables the extents on the VTKwindow.
        """
        self.store(self.Extents)
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def repr(self):
        """
        Return python scripting content.
        """
        output = dict()
        scale = [self.ScaleX.value(), self.ScaleY.value(), self.ScaleZ.value()]
        if any([s != 1 for s in scale]):
            options, sub_options = self._transform.options().toScriptString()
            output['filters'] = ['transform = chigger.filters.TransformFilter()']
            output['filters'] += ['transform.setOptions({})'.format(', '.join(options))]
            for key, value in sub_options.items():
                output['filters'] += ['transform.setOptions({}, {})'.format(repr(key), ', '.join(value))]

        return output


def main(size=None):
    """
    Run the MeshPlugin all by its lonesome.
    """
    from peacock.ExodusViewer.ExodusPluginManager import ExodusPluginManager
    from peacock.ExodusViewer.plugins.VTKWindowPlugin import VTKWindowPlugin
    from peacock.ExodusViewer.plugins.FilePlugin import FilePlugin
    widget = ExodusPluginManager(plugins=[lambda: VTKWindowPlugin(size=size), FilePlugin, MeshPlugin])
    widget.show()

    return widget, widget.VTKWindowPlugin

if __name__ == '__main__':
    from peacock.utils import Testing
    app = QtWidgets.QApplication(sys.argv)
    filenames = Testing.get_chigger_input_list('mug_blocks_out.e', 'displace.e')
    #filenames = Testing.get_chigger_input_list('diffusion_1.e', 'diffusion_2.e')
    widget, window = main(size=[600,600])
    widget.FilePlugin.onSetFilenames(filenames)
    sys.exit(app.exec_())
