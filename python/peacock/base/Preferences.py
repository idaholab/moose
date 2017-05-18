from PyQt5 import QtWidgets, QtCore
from collections import OrderedDict

class BasePreferenceWidget(object):
    """
    Base class for holding a single preference.
    """
    def __init__(self, name, widget, label, tooltip, default):
        self._name = name
        self._label = QtWidgets.QLabel(label)
        self._widget = widget
        self._widget.setToolTip(tooltip)
        self._default = default
        settings = QtCore.QSettings()
        self.load(settings)

    def load(self, settings):
        """
        Set the value of the widget given what is in the stored settings.
        """
        val = settings.value(self._name)
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
        settings.setValue(self._name, self.getValue())

    def key(self):
        """
        returns key used for the QSettings value
        """
        return self._name;

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
    def __init__(self, name, caption, default, tooltip):
        checkbox = QtWidgets.QCheckBox()
        super(BoolPreferenceWidget, self).__init__(name, checkbox, caption, tooltip, default)

    def setValue(self, val):
        self._widget.setChecked(bool(val))

    def getValue(self):
        return self._widget.isChecked()

class IntPreferenceWidget(BasePreferenceWidget):
    """
    Holds an integer prefernce, represented by a spin box.
    """
    def __init__(self, name, caption, default, min_val, max_val, tooltip):
        spin = QtWidgets.QSpinBox()
        spin.setMinimum(min_val)
        spin.setMaximum(max_val)
        super(IntPreferenceWidget, self).__init__(name, spin, caption, tooltip, default)

    def setValue(self, val):
        self._widget.setValue(int(val))

    def getValue(self):
        return self._widget.value()

class StringPreferenceWidget(BasePreferenceWidget):
    """
    Holds a string preference, represented by a line edit.
    """
    def __init__(self, name, caption, default, tooltip):
        line = QtWidgets.QLineEdit()
        super(StringPreferenceWidget, self).__init__(name, line, caption, tooltip, default)

    def setValue(self, val):
        self._widget.setText(str(val))

    def getValue(self):
        return self._widget.text()

class ComboPreferenceWidget(BasePreferenceWidget):
    """
    Holds a string preference from a list of options, represented by a combo box.
    """
    def __init__(self, name, caption, default, options, tooltip):
        combo = QtWidgets.QComboBox()
        combo.addItems(options)
        super(ComboPreferenceWidget, self).__init__(name, combo, caption, tooltip, default)

    def setValue(self, val):
        idx = self._widget.findText(str(val))
        if idx >= 0:
            self._widget.setCurrentIndex(idx)

    def getValue(self):
        return self._widget.currentText()

class Preferences(object):
    """
    Holds a list of widgets corresponding to settable preferences.
    """
    def __init__(self):
        super(Preferences, self).__init__()
        self._widgets = OrderedDict()

    def addWidget(self, widget):
        self._widgets[widget.key()] = widget

    def widgets(self):
        return self._widgets.values()

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
        self.addWidget(BoolPreferenceWidget(name, caption, default, tooltip))

    def addInt(self, name, caption, default, min_val, max_val, tooltip):
        """
        Convenience function to add an integer preference.
        """
        self.addWidget(IntPreferenceWidget(name, caption, default, min_val, max_val, tooltip))

    def addString(self, name, caption, default, tooltip):
        """
        Convenience function to add an string preference.
        """
        self.addWidget(StringPreferenceWidget(name, caption, default, tooltip))

    def addCombo(self, name, caption, default, options, tooltip):
        """
        Convenience function to add an string preference from a list of options.
        """
        self.addWidget(ComboPreferenceWidget(name, caption, default, options, tooltip))
