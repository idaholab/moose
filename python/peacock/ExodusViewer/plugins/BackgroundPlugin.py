import sys
import chigger
from PyQt5 import QtCore, QtGui, QtWidgets
from ExodusPlugin import ExodusPlugin

class BackgroundPlugin(QtWidgets.QGroupBox, ExodusPlugin):
    """
    Plugin responsible for background and labels
    """

    #: Emitted when the window needs updated.
    windowRequiresUpdate = QtCore.pyqtSignal()

    #: pyqtSignal: Emitted when the chigger objects options are changed
    windowOptionsChanged = QtCore.pyqtSignal(dict)

    #: pyqtSignal: Add/remove result objects
    appendResult = QtCore.pyqtSignal(chigger.base.ChiggerResultBase)
    removeResult = QtCore.pyqtSignal(chigger.base.ChiggerResultBase)

    def __init__(self):
        super(BackgroundPlugin, self).__init__()

        # Default colors
        self._top = QtGui.QColor(111, 111, 111)
        self._bottom = QtGui.QColor(180, 180, 180)

        # Setup this widget
        self.MainLayout = QtWidgets.QGridLayout()
        self.setTitle('Background and Labels:')
        self.setLayout(self.MainLayout)

        # Top
        self.TopLabel = QtWidgets.QLabel('Background Top:')
        self.TopButton = QtWidgets.QPushButton()
        self.GradientToggle = QtWidgets.QCheckBox('Background Gradient')
        self.MainLayout.addWidget(self.TopLabel, 1, 0)
        self.MainLayout.addWidget(self.TopButton, 1, 1)
        self.MainLayout.addWidget(self.GradientToggle, 0, 0)

        # Bottom
        self.BottomLabel = QtWidgets.QLabel('Background Bottom:')
        self.BottomButton = QtWidgets.QPushButton()
        self.MainLayout.addWidget(self.BottomLabel, 2, 0)
        self.MainLayout.addWidget(self.BottomButton, 2, 1)

        # Extents
        self.Extents = QtWidgets.QCheckBox('Extents')
        self.MainLayout.addWidget(self.Extents, 0, 3)

        # Node/Element labels
        self.Elements = QtWidgets.QCheckBox('Label Elements')
        self.MainLayout.addWidget(self.Elements, 1, 3)
        self.Nodes = QtWidgets.QCheckBox('Label Nodes')
        self.MainLayout.addWidget(self.Nodes, 2, 3)
        self.Values = QtWidgets.QCheckBox('Label Values')
        self.MainLayout.addWidget(self.Values, 3, 3)

        # Storage for Chigger objects that are toggled by this plugin
        self._cell_labels = None
        self._node_labels = None
        self._variable_labels = None
        self._volume_axes = None
        self.setup()

    def color(self):
        """
        Apply the supplied colors to the window.
        """

        top = self._top.getRgb()
        self.TopButton.setStyleSheet('border:none; background:rgb' + str(top))

        bottom = self._bottom.getRgb()
        self.BottomButton.setStyleSheet('border:none; background:rgb' + str(bottom))

        if self._window:
            if self.GradientToggle.isChecked():
                background = [bottom[0]/255., bottom[1]/255., bottom[2]/255.]
                background2 = [top[0]/255., top[1]/255., top[2]/255.]
            else:
                background = [top[0]/255., top[1]/255., top[2]/255.]
                background2 = None
            self.windowOptionsChanged.emit({'background':background, 'background2':background2, 'gradient_background':self.GradientToggle.isChecked()})
            self.windowRequiresUpdate.emit()

    def onWindowCreated(self, *args):
        """
        When the window is created apply the color.
        """
        super(BackgroundPlugin, self).onWindowCreated(*args)
        self.color()

    def _setupGradientToggle(self, qobject):
        """
        Setup method for gradient toggle.
        """
        qobject.setChecked(QtCore.Qt.Checked)
        qobject.clicked.connect(self._callbackGradientToggle)

    def _callbackGradientToggle(self, value):
        """
        Called when the gradient toggle is checked/Unchecked.
        """

        if value:
            self.TopLabel.setText('Background Top:')
            self.BottomLabel.setVisible(True)
            self.BottomButton.setVisible(True)
        else:
            self.TopLabel.setText('Background Color:')
            self.BottomLabel.setVisible(False)
            self.BottomButton.setVisible(False)
        self.color()

    def _setupTopButton(self, qobject):
        """
        Setup method for top color button.
        """
        self.TopLabel.setAlignment(QtCore.Qt.AlignRight)
        qobject.setStyleSheet('border:none;')
        qobject.setMaximumWidth(qobject.height())
        qobject.setAutoFillBackground(False)
        qobject.clicked.connect(self._callbackTopButton)

    def _callbackTopButton(self):
        """
        Callback for the top color dialog.
        """
        if self.GradientToggle.isChecked():
            title = 'Select top gradient color'
        else:
            title = 'Select solid color'

        dialog = QtWidgets.QColorDialog()
        c = dialog.getColor(initial=self._top, title=title)
        if c.isValid():
            self._top = c
            self.color()

    def _setupBottomButton(self, qobject):
        """
        Setup the bottom color button.
        """
        self.BottomLabel.setAlignment(QtCore.Qt.AlignRight)
        qobject.setStyleSheet('border:none;')
        qobject.setMaximumWidth(qobject.height())
        qobject.setAutoFillBackground(False)
        qobject.clicked.connect(self._callbackBottomButton)

    def _callbackBottomButton(self):
        """
        Callback for bottom color button.
        """
        dialog = QtWidgets.QColorDialog()
        c = dialog.getColor(initial=self._bottom, title='Select bottom gradient color')
        if c.isValid():
            self._bottom = c
            self.color()

    def _setupExtents(self, qobject):
        """
        Setup method for the extents toggle.
        """
        qobject.clicked.connect(self._callbackExtents)

    def _callbackExtents(self, value):
        """
        Enables/disables the extents on the VTKwindow.
        """
        if value:
            self._volume_axes = chigger.misc.VolumeAxes(self._result)
            self.appendResult.emit(self._volume_axes)
        else:
            self._volume_axes.reset()
            self.removeResult.emit(self._volume_axes)

        self.windowRequiresUpdate.emit()

    def _setupElements(self, qobject):
        """
        Setup method for the element labels.
        """
        qobject.clicked.connect(self._callbackElements)

    def _callbackElements(self, value):
        """
        Enables/disables the element labels.
        """
        if value:
            self._cell_labels = chigger.exodus.LabelExodusResult(self._result, label_type='cell', font_size=12)
            self.appendResult.emit(self._cell_labels)
        else:
            self._cell_labels.reset()
            self.removeResult.emit(self._cell_labels)
        self.windowRequiresUpdate.emit()

    def _setupNodes(self, qobject):
        """
        Setup method for the node labels.
        """
        qobject.clicked.connect(self._callbackNodes)

    def _callbackNodes(self, value):
        """
        Enables/disables the node labels.
        """
        if value:
            self._node_labels = chigger.exodus.LabelExodusResult(self._result, label_type='point', font_size=12)
            self.appendResult.emit(self._node_labels)
        else:
            self._node_labels.reset()
            self.removeResult.emit(self._node_labels)
        self.windowRequiresUpdate.emit()

    def _setupValues(self, qobject):
        """
        Setup method for the variable value labels.
        """
        qobject.clicked.connect(self._callbackValues)

    def _callbackValues(self, value):
        """
        Enables/disables the variable value labels.
        """
        if value:
            self._variable_labels = chigger.exodus.LabelExodusResult(self._result, label_type='variable', font_size=12)
            self.appendResult.emit(self._variable_labels)
        else:
            self._variable_labels.reset()
            self.removeResult.emit(self._variable_labels)

        self.windowRequiresUpdate.emit()


def main(size=None):
    """
    Run the ContourPlugin all by its lonesome.
    """
    from peacock.ExodusViewer.ExodusPluginManager import ExodusPluginManager
    from peacock.ExodusViewer.plugins.VTKWindowPlugin import VTKWindowPlugin
    widget = ExodusPluginManager(plugins=[lambda: VTKWindowPlugin(size=size), BackgroundPlugin])
    widget.show()

    return widget, widget.VTKWindowPlugin

if __name__ == '__main__':
    from peacock.utils import Testing
    app = QtWidgets.QApplication(sys.argv)
    filename = Testing.get_chigger_input('mug_blocks_out.e')
    widget, window = main()
    widget.initialize([filename])
    window.onResultOptionsChanged({'variable':'diffused'})
    window.onWindowRequiresUpdate()
    sys.exit(app.exec_())
