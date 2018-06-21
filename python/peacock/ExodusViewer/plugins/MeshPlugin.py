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
from ExodusPlugin import ExodusPlugin
import peacock
from peacock.utils import WidgetUtils

class MeshPlugin(peacock.base.PeacockCollapsibleWidget, ExodusPlugin):
    """
    Controls for the mesh appearance.
    """

    #: Emitted when window has changed
    windowRequiresUpdate = QtCore.pyqtSignal()

    #: pyqtSignal: Emitted when the chigger objects options are changed
    readerOptionsChanged = QtCore.pyqtSignal(dict)
    resultOptionsChanged = QtCore.pyqtSignal(dict)
    transformOptionsChanged = QtCore.pyqtSignal(dict)

    def __init__(self, **kwargs):
        peacock.base.PeacockCollapsibleWidget.__init__(self, collapsible_layout=QtWidgets.QVBoxLayout)
        ExodusPlugin.__init__(self, **kwargs)

        self._preferences.addBool("exodus/viewMesh",
                "View the mesh",
                False,
                "View the mesh by default",
                )

        # Current variable (used for caching settings)
        self._variable = None
        self._transform = chigger.filters.TransformFilter()

        # QGroupBox settings
        self.setTitle('Mesh')
        self.setSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Minimum)

        # Main Layout
        self.MainLayout = self.collapsibleLayout()

        # Displacements
        self.DisplacementToggle = QtWidgets.QCheckBox("Displacements")
        self.DisplacementMagnitude = QtWidgets.QDoubleSpinBox()

        # Mesh
        self.RepresentationLabel = QtWidgets.QLabel("Representation:")
        self.Representation = QtWidgets.QComboBox()
        self.ViewMeshToggle = QtWidgets.QCheckBox('View Mesh')

        # Scale
        self.ScaleLabel = QtWidgets.QLabel("Scale")
        self.ScaleXLabel = QtWidgets.QLabel("x:")
        self.ScaleYLabel = QtWidgets.QLabel("y:")
        self.ScaleZLabel = QtWidgets.QLabel("z:")
        self.ScaleX = QtWidgets.QDoubleSpinBox()
        self.ScaleY = QtWidgets.QDoubleSpinBox()
        self.ScaleZ = QtWidgets.QDoubleSpinBox()

        # Layouts
        self.DisplacementLayout = QtWidgets.QHBoxLayout()
        self.DisplacementLayout.addWidget(self.DisplacementToggle)
        self.DisplacementLayout.addWidget(self.DisplacementMagnitude)

        self.MeshViewLayout = QtWidgets.QHBoxLayout()
        self.MeshViewLayout.addWidget(self.RepresentationLabel)
        self.MeshViewLayout.addWidget(self.Representation)
        self.MeshViewLayout.addWidget(self.ViewMeshToggle)

        self.ScaleLayout = QtWidgets.QHBoxLayout()
        self.ScaleLayout.addWidget(self.ScaleLabel)
        self.ScaleLayout.addWidget(self.ScaleXLabel)
        self.ScaleLayout.addWidget(self.ScaleX)
        self.ScaleLayout.addWidget(self.ScaleYLabel)
        self.ScaleLayout.addWidget(self.ScaleY)
        self.ScaleLayout.addWidget(self.ScaleZLabel)
        self.ScaleLayout.addWidget(self.ScaleZ)

        self.MainLayout.addLayout(self.DisplacementLayout)
        self.MainLayout.addLayout(self.MeshViewLayout)
        self.MainLayout.addLayout(self.ScaleLayout)

        # Call widget setup methods
        self.setup()

    def onVariableChanged(self, *args):
        """
        When a variable changes, load the state of the clip.
        """
        if self.isEnabled():
            self.store(self.stateKey(self._variable), 'Variable')
        super(MeshPlugin, self).onVariableChanged(*args)
        self.load(self.stateKey(self._variable), 'Variable')
        if self._result:
            self.mesh()

    def onWindowCreated(self, reader, result, window):
        """
        Reload the mesh options when the window gets created
        """
        super(MeshPlugin, self).onWindowCreated(reader, result, window)
        self.mesh()

    def mesh(self):
        """
        Updates the results for the changes to the mesh view.
        """
        # Options to pass to ExodusResult
        reader_options = dict()
        result_options = dict()
        filters = []
        if self._result:
            filters = self._result.getOption('filters')

        # Displacement toggle and magnitude
        reader_options['displacements'] = bool(self.DisplacementToggle.isChecked())
        reader_options['displacement_magnitude'] = self.DisplacementMagnitude.value()
        self.DisplacementMagnitude.setEnabled(reader_options['displacements'])

        # Representation
        result_options['representation'] = str(self.Representation.currentText()).lower()

        # Mesh Toggle
        result_options['edges'] = bool(self.ViewMeshToggle.checkState())
        result_options['edge_color'] = [0,0,0]

        # Scale
        scale = [self.ScaleX.value(), self.ScaleY.value(), self.ScaleZ.value()]
        if any([s != 1 for s in scale]):
            self._transform.setOption('scale', scale)
            if self._transform not in filters:
                filters.append(self._transform)
        else:
            if self._transform in filters:
                filters.remove(self._transform)
        result_options['filters'] = filters

        # Emit the update signal with the new arguments
        self.readerOptionsChanged.emit(reader_options)
        self.resultOptionsChanged.emit(result_options)
        self.windowRequiresUpdate.emit()

    def _setupDisplacementToggle(self, qobject):
        """
        Setup method for DisplacementToggle widget. (protected)
        """
        qobject.setChecked(True)
        qobject.stateChanged.connect(lambda value: self.mesh())

    def _setupDisplacementMagnitude(self, qobject):
        """
        Setup for DisplacementMagnitude widget. (protected)
        """
        qobject.setSingleStep(0.1)
        qobject.setValue(1.0)
        qobject.valueChanged.connect(self.mesh)

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

        WidgetUtils.loadWidget(self.ViewMeshToggle, self.stateKey(self.Representation.currentIndex()), 'Respresentation')

        index = self.Representation.currentIndex()
        if index == 0:
            self.ViewMeshToggle.setEnabled(True)
        else:
            self.ViewMeshToggle.setEnabled(False)
            self.ViewMeshToggle.setChecked(QtCore.Qt.Unchecked)

        self.mesh()

    def _setupViewMeshToggle(self, qobject):
        """
        Setup for showing the ViewMeshToggle widget. (protected)
        """
        qobject.stateChanged.connect(lambda value: self._callbackViewMeshToggle())
        qobject.setChecked(self._preferences.value("exodus/viewMesh"))

    def _callbackViewMeshToggle(self):
        """
        Callback for ViewMeshToggle widget. (protected)
        """
        WidgetUtils.storeWidget(self.ViewMeshToggle, self.stateKey(self.Representation.currentIndex()), 'Respresentation')
        self.mesh()

    def _setupScaleLabel(self, qobject):
        """
        Helper method for adding various scale related widgets. (protected)
        """
        for id in ['X', 'Y', 'Z']:
            label = getattr(self, 'Scale' + id + 'Label')
            spinbox = getattr(self, 'Scale' + id)
            spinbox.setSingleStep(0.1)
            spinbox.setValue(1.0)
            spinbox.valueChanged.connect(self.mesh)
            spinbox.setMaximum(1000)
            label.setAlignment(QtCore.Qt.AlignRight | QtCore.Qt.AlignVCenter)

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
            for key, value in sub_options.iteritems():
                output['filters'] += ['transform.setOptions({}, {})'.format(repr(key), ', '.join(value))]

        return output


def main(size=None):
    """
    Run the MeshPlugin all by its lonesome.
    """
    from peacock.ExodusViewer.ExodusPluginManager import ExodusPluginManager
    from peacock.ExodusViewer.plugins.VTKWindowPlugin import VTKWindowPlugin
    widget = ExodusPluginManager(plugins=[lambda: VTKWindowPlugin(size=size), MeshPlugin])
    widget.show()

    return widget, widget.VTKWindowPlugin

if __name__ == '__main__':
    from peacock.utils import Testing
    app = QtWidgets.QApplication(sys.argv)
    widget, window = main()
    filename = Testing.get_chigger_input('mug_blocks_out.e')
    window.initialize([filename])
    sys.exit(app.exec_())
