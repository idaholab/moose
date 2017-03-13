import sys
from PyQt5 import QtCore, QtWidgets
import chigger
from ExodusPlugin import ExodusPlugin
from peacock.utils import WidgetUtils

class MeshPlugin(QtWidgets.QGroupBox, ExodusPlugin):
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
        super(MeshPlugin, self).__init__(**kwargs)

        # Current variable (used for caching settings)
        self._variable = None
        self._transform = chigger.filters.TransformFilter()

        # QGroupBox settings
        self.setTitle('Mesh')

        # Main Layout
        self.MainLayout = QtWidgets.QVBoxLayout()
        self.MainLayout.setContentsMargins(0, 10, 0, 10)
        self.setLayout(self.MainLayout)

        # Displacements
        self.DisplacementToggle = QtWidgets.QCheckBox("Displacements")
        self.DisplacmentMagnitude = QtWidgets.QDoubleSpinBox()

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
        self.DisplacementLayout.addWidget(self.DisplacmentMagnitude)

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
        super(MeshPlugin, self).onVariableChanged(*args)
        self.load(self._variable, 'Variable')
        if self._result:
            self.mesh()

    def mesh(self):
        """
        Updates the results for the changes to the mesh view.
        """
        # Options to pass to ExodusResult
        reader_options = dict()
        result_options = dict()
        filters = self._result.getOption('filters')

        # Displacement toggle and magnitude
        reader_options['displacements'] = bool(self.DisplacementToggle.checkState())
        reader_options['displacement_magnitude'] = self.DisplacmentMagnitude.value()
        self.DisplacmentMagnitude.setEnabled(reader_options['displacements'])

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
        self.store(self._variable, 'Variable')
        self.windowRequiresUpdate.emit()

    def _setupDisplacementToggle(self, qobject):
        """
        Setup method for DisplacementToggle widget. (protected)
        """
        qobject.setChecked(True)
        qobject.clicked.connect(self.mesh)

    def _setupDisplacmentMagnitude(self, qobject):
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

        WidgetUtils.loadWidget(self.ViewMeshToggle, self.Representation.currentIndex(), 'Respresentation')

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
        qobject.clicked.connect(self._callbackViewMeshToggle)

    def _callbackViewMeshToggle(self):
        """
        Callback for ViewMeshToggle widget. (protected)
        """
        WidgetUtils.storeWidget(self.ViewMeshToggle, self.Representation.currentIndex(), 'Respresentation')
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
