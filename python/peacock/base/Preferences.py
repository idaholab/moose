#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5 import QtWidgets, QtCore, QtGui
from collections import OrderedDict

class BasePreferenceWidget(QtWidgets.QWidget):
    """
    Base class for holding a single preference.
    """

    # A generic signal capable of handling any python type
    valueSaved = QtCore.pyqtSignal(object)

    def __init__(self, name, widget, label, tooltip, default, prekey=""):
        super(BasePreferenceWidget, self).__init__()
        self._name = name
        self._label = QtWidgets.QLabel(label)
        self._widget = widget
        self._widget.setToolTip(tooltip)
        self._default = default
        self._prekey = prekey
        settings = QtCore.QSettings()
        self.load(settings)

    def load(self, settings):
        """
        Set the value of the widget given what is in the stored settings.
        """
        val = settings.value(self.key())
        if val is None:
            self.setValue(self._default)
        else:
            self.setValue(val)

    def getValue(self):
        """
        Function that child classes need to inherit that returns the value of the widget
        """

    def setValue(self, val):
        """
        Function that child classes need to inherit that sets the value of the widget
        """

    def save(self, settings):
        """
        Saves the value of the widget to disk
        """
        settings.setValue(self.key(), self.getValue())
        self.valueSaved.emit(self.getValue())

    def key(self):
        """
        returns key used for the QSettings value
        """
        return self._prekey + self._name;

    def label(self):
        """
        Returns the label widget
        """
        return self._label

    def widget(self):
        """
        Returns the widget that holds the preference value
        """
        return self._widget

class BoolPreferenceWidget(BasePreferenceWidget):
    """
    Holds a boolean preference, represented by a checkbox.
    """
    def __init__(self, name, caption, default, tooltip, prekey):
        checkbox = QtWidgets.QCheckBox()
        super(BoolPreferenceWidget, self).__init__(name, checkbox, caption, tooltip, default, prekey)

    def setValue(self, val):
        self._widget.setChecked(bool(val))

    def getValue(self):
        return self._widget.isChecked()

class IntPreferenceWidget(BasePreferenceWidget):
    """
    Holds an integer preference, represented by a spin box.
    """
    def __init__(self, name, caption, default, min_val, max_val, tooltip, prekey):
        spin = QtWidgets.QSpinBox()
        spin.setMinimum(min_val)
        spin.setMaximum(max_val)
        super(IntPreferenceWidget, self).__init__(name, spin, caption, tooltip, default, prekey)

    def setValue(self, val):
        self._widget.setValue(int(val))

    def getValue(self):
        return self._widget.value()

class StringPreferenceWidget(BasePreferenceWidget):
    """
    Holds a string preference, represented by a line edit.
    """
    def __init__(self, name, caption, default, tooltip, prekey):
        line = QtWidgets.QLineEdit()
        super(StringPreferenceWidget, self).__init__(name, line, caption, tooltip, default, prekey)

    def setValue(self, val):
        self._widget.setText(str(val))

    def getValue(self):
        return self._widget.text()

class ComboPreferenceWidget(BasePreferenceWidget):
    """
    Holds a string preference from a list of options, represented by a combo box.
    """
    def __init__(self, name, caption, default, options, tooltip, prekey):
        combo = QtWidgets.QComboBox()
        combo.addItems(options)
        super(ComboPreferenceWidget, self).__init__(name, combo, caption, tooltip, default, prekey)

    def setValue(self, val):
        idx = self._widget.findText(str(val))
        if idx >= 0:
            self._widget.setCurrentIndex(idx)

    def getValue(self):
        return self._widget.currentText()

class ColorPreferenceWidget(BasePreferenceWidget):
    """
    Holds a color preference, represented by a button that opens a QColorDialog
    """
    def __init__(self, name, caption, default, tooltip, prekey):
        button = QtWidgets.QPushButton()
        self._color = default.name()
        button.pressed.connect(self._chooseColor)
        super(ColorPreferenceWidget, self).__init__(name, button, caption, tooltip, self._color, prekey)

    def setValue(self, val):
        self._color = val
        c = QtGui.QColor(val)
        rgb = c.getRgb()
        self._widget.setStyleSheet('border:none; background:rgb' + str(rgb))

    def getValue(self):
        return self._color

    def _chooseColor(self):
        color = QtGui.QColor(self._color)
        c = QtWidgets.QColorDialog.getColor(color)
        if c.isValid():
            self._color = c.name()
            self.setValue(self._color)

class Preferences(object):
    """
    Holds a list of widgets corresponding to settable preferences.
    """
    def __init__(self, key=""):
        super(Preferences, self).__init__()
        self._widgets = OrderedDict()
        self._key = key
        if key:
            self._key += "/"

    def addWidget(self, widget):
        self._widgets[widget._name] = widget

    def widgets(self):
        return list(self._widgets.values())

    def widget(self, key):
        return self._widgets[key]

    def value(self, name):
        w = self._widgets.get(name)
        if w:
            return w.getValue()

    def addBool(self, name, caption, default, tooltip):
        """
        Convenience function to add a boolean preference.
        """
        self.addWidget(BoolPreferenceWidget(name, caption, default, tooltip, self._key))

    def addInt(self, name, caption, default, min_val, max_val, tooltip):
        """
        Convenience function to add an integer preference.
        """
        self.addWidget(IntPreferenceWidget(name, caption, default, min_val, max_val, tooltip, self._key))

    def addString(self, name, caption, default, tooltip):
        """
        Convenience function to add an string preference.
        """
        self.addWidget(StringPreferenceWidget(name, caption, default, tooltip, self._key))

    def addCombo(self, name, caption, default, options, tooltip):
        """
        Convenience function to add an string preference from a list of options.
        """
        self.addWidget(ComboPreferenceWidget(name, caption, default, options, tooltip, self._key))

    def addColor(self, name, caption, default, tooltip):
        """
        Convenience function to add a color preference.
        """
        self.addWidget(ColorPreferenceWidget(name, caption, default, tooltip, self._key))
