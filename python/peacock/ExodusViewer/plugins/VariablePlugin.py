import os
import sys
from PyQt5 import QtCore, QtWidgets, QtGui
import chigger
import mooseutils
import glob
from peacock.utils import WidgetUtils
from ExodusPlugin import ExodusPlugin

class VariablePlugin(QtWidgets.QGroupBox, ExodusPlugin):
    """
    Class for controlling variable, range, and colormaps.
    """

    #: pyqtSignal: Emitted with the variable is changed
    variableChanged = QtCore.pyqtSignal(str)

    #: pyqtSignal: Emitted when the window needs to be updated
    windowRequiresUpdate = QtCore.pyqtSignal()

    #: pyqtSignal: Emitted when the chigger objects options are changed
    readerOptionsChanged = QtCore.pyqtSignal(dict)
    resultOptionsChanged = QtCore.pyqtSignal(dict)

    #: pyqtSignal: Emitted when colorbar is added/removed
    appendResult = QtCore.pyqtSignal(chigger.exodus.ExodusColorBar)
    removeResult = QtCore.pyqtSignal(chigger.exodus.ExodusColorBar)

    def __init__(self, **kwargs):
        super(VariablePlugin, self).__init__(**kwargs)

        # Cache for auto limits
        self._auto = [True, True]
        self._colorbar = None

        # QGroupBox settings
        self.setTitle('Variable:')

        self.MainLayout = QtWidgets.QVBoxLayout()
        self.MainLayout.setContentsMargins(0, 10, 0, 10)
        self.setLayout(self.MainLayout)

        # Variable selection
        self.VariableListLayout = QtWidgets.QHBoxLayout()
        self.VariableList = QtWidgets.QComboBox()
        self.VariableList.setFocusPolicy(QtCore.Qt.StrongFocus)
        self.VariableListLayout.addWidget(self.VariableList)

        # Component selection
        self.ComponentList = QtWidgets.QComboBox()
        self.ComponentList.setFocusPolicy(QtCore.Qt.StrongFocus)
        self.VariableListLayout.addWidget(self.ComponentList)

        # Min. value selection
        self.RangeMinimumLabel = QtWidgets.QLabel("Minimum:")
        self.RangeMinimum = QtWidgets.QLineEdit()

        # Max. value selection
        self.RangeMaximumLabel = QtWidgets.QLabel("Maximum:")
        self.RangeMaximum = QtWidgets.QLineEdit()

        # Colormap
        self.ColorMapList = QtWidgets.QComboBox()
        self.ColorMapList.setFocusPolicy(QtCore.Qt.StrongFocus)
        self.ReverseColorMap = QtWidgets.QCheckBox("Reverse Colormap")

        # Colorbar toggle
        self.ColorBarToggle = QtWidgets.QCheckBox("Colorbar")

        # Layout for the range controls
        self.GridLayout = QtWidgets.QGridLayout()
        self.GridLayout.addWidget(self.RangeMinimumLabel, 0, 0)
        self.GridLayout.addWidget(self.RangeMinimum, 0, 1)
        self.GridLayout.addWidget(self.RangeMaximumLabel, 1, 0)
        self.GridLayout.addWidget(self.RangeMaximum, 1, 1)
        self.GridLayout.addWidget(self.ColorMapList, 2, 0, 1, 2)
        self.GridLayout.addWidget(self.ReverseColorMap, 3, 0, 1, 2)
        self.GridLayout.addWidget(self.ColorBarToggle, 4, 0, 1, 2)

        # Add items to the main layout, in proper order
        self.MainLayout.addLayout(self.VariableListLayout)
        self.MainLayout.addLayout(self.GridLayout)

        # Call widget setup methods
        self.setup()

    def onFileChanged(self, *args):
        super(VariablePlugin, self).onFileChanged(*args)
        self.load(self._filename, 'Filename')

    def onWindowUpdated(self, *args):
        """
        Update the variable list when the window is updated.
        """
        if self._reader:
            self.VariableList.blockSignals(True)
            self.__setVariableList(self._reader)
            self.VariableList.blockSignals(False)

    def onWindowCreated(self, reader, result, window):
        """
        Initializes the GUI, called after the data is loaded via VTKWindowPlugin

        Args:
            reader[chigger.ExodusReader]: The current reader object.
            result[chigger.ExodusResult]: The current result object.
        """
        super(VariablePlugin, self).onWindowCreated(reader, result, window)

        # Create the colorbar
        self._colorbar = chigger.exodus.ExodusColorBar(result)

        # Populate variable list with nodal/elemental variables
        self.VariableList.blockSignals(True)
        self.__setVariableList(reader)

        # Update the settings based on current selections
        self.ColorBarToggle.clicked.emit(self.ColorBarToggle.isChecked())
        self.ColorMapList.currentIndexChanged.emit(self.ColorMapList.currentIndex())
        self.ReverseColorMap.clicked.emit(self.ReverseColorMap.isChecked())

        # Select the variable
        self.VariableList.blockSignals(False)
        self.VariableList.currentIndexChanged.emit(self.VariableList.currentIndex())

    def onTimeChanged(self):
        """
        Update the limits when the time changes.
        """
        try:
            self.setLimit(0, emit=False)
            self.setLimit(1, emit=False)
        except Exception:
            mooseutils.mooseDebug('Failed to set limits, likely due to corrupt Exodus file.', color='RED', traceback=True)

    def setLimit(self, index, emit=True):
        """
        Callback for min/max editing. This controls the auto flags that allow for the compute limits to display.

        Args:
            qobject[QWidget]: The object being adjusted
        """
        # Determine the object
        qobject = [self.RangeMinimum, self.RangeMaximum][index]
        key = ['min', 'max'][index]
        component = self.ComponentList.currentData()

        # Do nothing if the object is being editted
        if qobject.hasFocus():
            return

        # Extract the text in the edit box
        text = qobject.text()

        if self._result:
            # Empty: Display the limits in grey
            if (text == '' ) or (self._auto[index] == True):

                #@TODO: This update call should not be needed, but somewhere the result is getting out-of-date
                if self._result.needsUpdate():
                    self._result.update()
                lim = self._result.getRange()
                self.resultOptionsChanged.emit({key:lim[index]})
                qobject.setText(str(lim[index]))
                qobject.setStyleSheet('color:#8C8C8C')
                self._auto[index] = True
            else:
                try:
                    self.resultOptionsChanged.emit({key:float(text)})
                    self._auto[index] = False
                except:
                    qobject.setStyleSheet('color:#ff0000')

        self.store(self._filename, 'Filename')
        WidgetUtils.storeWidget(qobject, component, 'Component')
        if emit:
            self.windowRequiresUpdate.emit()

    def _setupVariableList(self, qobject):
        """
        Setup method for variable selection.
        """
        qobject.currentIndexChanged.connect(self._callbackVariableList)

    @QtCore.pyqtSlot(int)
    def _callbackVariableList(self, index):
        """
        Called when a variable is selected.
        """

        # Get the current variable from the list, do nothing if it is empty
        self._variable = str(self.VariableList.currentText())
        if not self._variable:
            return

        # Toggle visibility of Component list
        data = self.VariableList.itemData(index)
        if data.num_components == 1:
            self.ComponentList.setCurrentIndex(0)
            self.ComponentList.setEnabled(False)
        else:
            self.ComponentList.setEnabled(True)

        # Emit the variable changed signal
        self.readerOptionsChanged.emit({'variables':[self._variable]})
        self.resultOptionsChanged.emit({'variable':self._variable})
        self.load(self._variable, 'Variable')

        # Set limits if the file exists
        self.windowRequiresUpdate.emit()
        self.setLimit(0)
        self.setLimit(1)
        self.variableChanged.emit(self._variable)

    def _setupComponentList(self, qobject):
        """
        Setup for component selection.
        """
        qobject.addItem('Magnitude', -1)
        qobject.addItem('x', 0)
        qobject.addItem('y', 1)
        qobject.addItem('z', 2)
        qobject.setEnabled(False)
        self.ComponentList.currentIndexChanged.connect(self._callbackComponentList)

    def _callbackComponentList(self):
        """
        Called when the component is selected.
        """
        index = self.ComponentList.currentData()
        self.resultOptionsChanged.emit({'component':index})
        self.store(self._variable, 'Variable')
        WidgetUtils.loadWidget(self.RangeMinimum, index, 'Component')
        WidgetUtils.loadWidget(self.RangeMaximum, index, 'Component')
        self.setLimit(0)
        self.setLimit(1)
        self.windowRequiresUpdate.emit()

    def _setupRangeMinimum(self, qobject):
        """
        Setup the range minimum editing.
        """
        qobject.editingFinished.connect(lambda: self.setLimit(0))
        qobject.returnPressed.connect(qobject.clearFocus)
        qobject.textChanged.connect(lambda: self._callbackBlack(qobject, 0))

    def _setupRangeMaximum(self, qobject):
        """
        Setup the range minimum editing.
        """
        qobject.editingFinished.connect(lambda: self.setLimit(1))
        qobject.returnPressed.connect(qobject.clearFocus)
        qobject.textChanged.connect(lambda: self._callbackBlack(qobject, 1))

    def _callbackBlack(self, qobject, index):
        """
        Set the min/max text to black when it is being edited.
        """
        qobject.setStyleSheet('color:#000000')
        self._auto[index] = False

    def _setupColorMapList(self, qobject):
        """
        Setup the list of colormaps.
        """
        filenames = glob.glob(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', 'icons', 'colormaps', '*.png')))
        for i in range(len(filenames)):
            name = os.path.basename(filenames[i])[0:-4]
            self.ColorMapList.addItem(name)
            self.ColorMapList.setItemIcon(i, QtGui.QIcon(filenames[i]))
            if name == 'default':
                self.ColorMapList.setCurrentIndex(i)

        qobject.currentIndexChanged.connect(self._callbackColorMapList)

    def _callbackColorMapList(self):
        """
        Callback for colormap selection.
        """
        self.resultOptionsChanged.emit({'cmap': str(self.ColorMapList.currentText())})
        self.windowRequiresUpdate.emit()

    def _setupReverseColorMap(self, qobject):
        """
        Setup the reverse toggle.
        """
        qobject.clicked.connect(self._callbackReverseColorMap)

    def _callbackReverseColorMap(self, value):
        """
        Callback for reverse toggle.
        """
        self.resultOptionsChanged.emit({'cmap_reverse':value})
        self.windowRequiresUpdate.emit()

    def _setupColorBarToggle(self, qobject):
        """
        Setup the colorbar toggle.
        """
        qobject.setChecked(True)
        qobject.clicked.connect(self._callbackColorBarToggle)

    def _callbackColorBarToggle(self, value):
        """
        Callback for the colorbar toggle.
        """
        if value:
            self.appendResult.emit(self._colorbar)
        else:
            self.removeResult.emit(self._colorbar)
        self.windowRequiresUpdate.emit()

    def __setVariableList(self, reader):
        """
        Helper for updating the variable list while maintaining the current selection
        """
        variables = self._reader.getVariableInformation(var_types=[self._reader.NODAL, self._reader.ELEMENTAL])
        current = self.VariableList.currentText()
        self.VariableList.clear()
        for vinfo in variables.itervalues():
            self.VariableList.addItem(vinfo.name, vinfo)

        # Set the current variable
        idx = self.VariableList.findText(current)
        if idx > -1:
            self.VariableList.setCurrentIndex(idx)
        else:
            self.VariableList.setCurrentIndex(0)

def main(size=None):
    """
    Run the VTKFilePlugin all by its lonesome.
    """
    from peacock.ExodusViewer.ExodusPluginManager import ExodusPluginManager
    from VTKWindowPlugin import VTKWindowPlugin
    from FilePlugin import FilePlugin
    widget = ExodusPluginManager(plugins=[lambda: VTKWindowPlugin(size=size), FilePlugin, VariablePlugin])
    widget.show()

    return widget, widget.VTKWindowPlugin

if __name__ == '__main__':
    from peacock.utils import Testing
    app = QtWidgets.QApplication(sys.argv)
    filenames = Testing.get_chigger_input_list('mug_blocks_out.e', 'vector_out.e', 'displace.e')
    widget, _ = main()
    widget.initialize(filenames)
    sys.exit(app.exec_())
