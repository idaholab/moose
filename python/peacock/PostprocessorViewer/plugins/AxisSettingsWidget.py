#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5 import QtCore, QtWidgets
import peacock
import mooseutils

class AxisSettingsWidget(peacock.base.MooseWidget, QtWidgets.QWidget):
    """
    Widget for axis settings.
    """

    axesModified = QtCore.pyqtSignal()

    def __init__(self, name, index):
        super(AxisSettingsWidget, self).__init__()

        self._name = name
        self._axes = None
        self._index = index # used for repr()
        self._default_label = ''
        self._auto = [True, True]

        self.MainLayout = QtWidgets.QVBoxLayout()
        self.setLayout(self.MainLayout)
        self.setSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Fixed)

        # Axis label
        self.LabelLayout = QtWidgets.QHBoxLayout()
        self.LabelLabel = QtWidgets.QLabel("Label:")
        self.Label = QtWidgets.QLineEdit(self)
        self.LabelLayout.addWidget(self.LabelLabel)
        self.LabelLayout.addWidget(self.Label)

        # Axis minimum
        self.RangeMinimumLayout = QtWidgets.QHBoxLayout()
        self.RangeMinimumLabel = QtWidgets.QLabel("Minimum:")
        self.RangeMinimum = QtWidgets.QLineEdit()
        self.RangeMinimumLayout.addWidget(self.RangeMinimumLabel)
        self.RangeMinimumLayout.addWidget(self.RangeMinimum)

        # Axis maximum
        self.RangeMaximumLayout = QtWidgets.QHBoxLayout()
        self.RangeMaximumLabel = QtWidgets.QLabel("Maximum:")
        self.RangeMaximum = QtWidgets.QLineEdit()
        self.RangeMaximumLayout.addWidget(self.RangeMaximumLabel)
        self.RangeMaximumLayout.addWidget(self.RangeMaximum)

        # Grid lines
        self.GridLayout = QtWidgets.QHBoxLayout()
        self.GridToggle = QtWidgets.QCheckBox('Grid')
        self.GridLayout.addWidget(self.GridToggle)

        # Scale selection
        self.Scale = QtWidgets.QCheckBox('Log Scale')
        self.GridLayout.addWidget(self.Scale)
        self.GridLayout.addStretch()

        self.MainLayout.addLayout(self.LabelLayout)
        self.MainLayout.addLayout(self.RangeMinimumLayout)
        self.MainLayout.addLayout(self.RangeMaximumLayout)
        self.MainLayout.addLayout(self.GridLayout)

        self.MainLayout.addStretch(1)

        self.setup()

    @QtCore.pyqtSlot(list)
    def setLabelDefault(self, default):
        """
        Receives the default axis label.
        """
        self._default_label = '; '.join(default)

    def onFigureCreated(self, axes):
        """
        Set the Axes object that this widget operates.
        """
        self._axes = axes

    def set(self, function, *args, **kwargs):
        """
        Helper method for calling set methods containing the 'x' or 'y' (e.g., "set_xscale")
        """
        if self._axes == None:
            import traceback; traceback.print_stack()

        func = getattr(self._axes, function.format(self._name))
        func(*args, **kwargs)

    def get(self, function):
        """
        Helper method for calling get methods containing 'x' or 'y' (e.g., "get_xlim")
        """
        func = getattr(self._axes, function.format(self._name))
        return func()

    def repr(self):
        """
        Returns the strings for building script
        """

        # Do nothing if no data is plotted on the axes
        if not self._axes.has_data():
            return [], []

        output = ['', '# {}{}-axis Settings'.format(self._name, self._index)]

        # Label
        output += ["axes{}.set_{}label({})".format(self._index, self._name, repr(str(self.get('get_{}label'))))]

        # Grid
        if self.GridToggle.isChecked():
            output += ["axes{}.grid(True, axis={})".format(self._index, repr(self._name))]

        # Scale
        if self.Scale.isChecked():
            output += ["axes{}.set_{}scale('log')".format(self._index, self._name)]

        # Limits
        lims = self.get('get_{}lim')
        output += ["axes{}.set_{}lim({})".format(self._index, self._name, repr(list(lims)))]

        return output, []

    def setLimit(self, index):
        """
        Callback for min/max editing. This controls the auto flags that allow for the compute limits to display.
        """

        # Determine the object
        qobject = [self.RangeMinimum, self.RangeMaximum][index]

        # Extract the text in the edit box
        text = qobject.text()

        # Empty: Display the limits in grey
        if (self._auto[index] == True) and (not self._axes.has_data()):
            qobject.setText('')

        elif (text == '') or (self._auto[index] == True):
            self.set('set_{}lim', auto=True)
            lim = self.get('get_{}lim')
            qobject.setText(str(lim[index]))
            qobject.setStyleSheet('color:#8C8C8C')
            self._auto[index] = True
        else:
            try:
                key = "{}{}".format(self._name, ["min", "max"][index])
                num = float(text)
                options = {key:num}
                self.set('set_{}lim', **options)
                self._auto[index] = False
            except:
                qobject.setStyleSheet('color:#ff0000')

        self.axesModified.emit()

    def _setupLabel(self, qobject):
        """
        Setup method for the label edit.
        """
        qobject.editingFinished.connect(self._callbackLabel)

    def _callbackLabel(self):
        """
        Execute when the label is changed.
        """
        label = self.Label.text()
        if not label:
            label = self._default_label
        self.set('set_{}label', label)
        self.axesModified.emit()

    def _setupRangeMinimum(self, qobject):
        """
        Setup method minimum setting.
        """
        qobject.editingFinished.connect(lambda: self.setLimit(0))
        qobject.textChanged.connect(lambda: self._callbackBlack(qobject, 0))

    def _setupRangeMaximum(self, qobject):
        """
        Setup for the maximum setting.
        """
        qobject.editingFinished.connect(lambda: self.setLimit(1))
        qobject.textChanged.connect(lambda: self._callbackBlack(qobject, 1))

    def _callbackBlack(self, qobject, index):
        """
        Set the min/max text to black when it is being editted.
        """
        qobject.setStyleSheet('color:#000000')
        self._auto[index] = False

    def _setupGridToggle(self, qobject):
        """
        Setup method for grid line toggle.
        """
        qobject.clicked.connect(self._callbackGridToggle)

    def _callbackGridToggle(self, value):
        """
        Callback for grid line toggle.
        """
        self._axes.grid(value, axis=self._name)
        self.axesModified.emit()

    def _setupScale(self, qobject):
        """
        Setup method for scale toggle.
        """
        qobject.clicked.connect(self._callbackScale)

    def _callbackScale(self, value):
        """
        Callback for scale toggle.
        """
        if value:
            try:
                self.set('set_{}scale', 'log')
            except:
                mooseutils.mooseError('Failed to set log axis limits, your data likely crosses zero.')
                self.Scale.setCheckState(QtCore.Qt.Unchecked)
                self.set('set_{}scale', 'linear')
        else:
            self.set('set_{}scale', 'linear')
        self.axesModified.emit()
