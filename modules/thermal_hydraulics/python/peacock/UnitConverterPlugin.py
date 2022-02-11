#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import sys
from PyQt5 import QtCore, QtWidgets, QtGui
import peacock
import UnitConversion


class UnitConverterPlugin(QtWidgets.QWidget):
    """
    Unit conversion plugin
    """

    def __init__(self, **kwargs):
        super(UnitConverterPlugin, self).__init__(**kwargs)

        # default units (used in THM/RELAP-7/etc.)
        self._default_units = {
            'Pressure': 'Pascal',
            'Mass': 'Kilogram',
            'Volume': 'Cubic meter',
            'Speed': 'Meter per second',
            'Length': 'Meter',
            'Temperature': 'Kelvin',
            'Energy': 'Joule per kilogram',
            'Mass flow rate': 'Kilogram per second',
            'Time': 'Second'
        }

        self.setSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Preferred)

        self.LayoutMain = QtWidgets.QGridLayout(self)
        self.LayoutMain.setVerticalSpacing(2)

        self.SearchEdit = QtWidgets.QLineEdit()
        self.SearchEdit.setPlaceholderText("Example: 30 C in K")
        self.SearchEdit.setClearButtonEnabled(True)
        self.LayoutMain.addWidget(self.SearchEdit, 0, 0, 1, 3)

        self.LayoutMain.addItem(QtWidgets.QSpacerItem(8, 8, QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed), 1, 0, 1, 3)

        self.UnitGroup = QtWidgets.QComboBox()
        for group in UnitConversion.GROUPS:
            self.UnitGroup.addItem(group.name(), group)
        self.LayoutMain.addWidget(self.UnitGroup, 2, 0, 1, 3)

        self.LayoutMain.addItem(QtWidgets.QSpacerItem(4, 4, QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed), 3, 0, 1, 3)

        self.InValue = QtWidgets.QLineEdit()
        self.InValue.setValidator(QtGui.QDoubleValidator())
        self.LayoutMain.addWidget(self.InValue, 4, 0)

        self.InUnit = QtWidgets.QComboBox()
        self.LayoutMain.addWidget(self.InUnit, 5, 0)

        self.LabelEqual = QtWidgets.QLabel("=")
        self.LayoutMain.addWidget(self.LabelEqual, 4, 1)

        self.OutValue = QtWidgets.QLineEdit()
        self.OutValue.setValidator(QtGui.QDoubleValidator())
        self.LayoutMain.addWidget(self.OutValue, 4, 2)

        self.OutUnit = QtWidgets.QComboBox()
        self.LayoutMain.addWidget(self.OutUnit, 5, 2)

        self.LayoutMain.addItem(QtWidgets.QSpacerItem(1, 0, QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding), 6, 0, 1, 3)

        self.SearchEdit.textChanged.connect(self.onSearchEditModified)
        self.UnitGroup.currentIndexChanged.connect(self.onUnitGroupChanged)
        self.InValue.textChanged.connect(self.onInValueModified)
        self.InUnit.currentIndexChanged.connect(self.onInUnitChanged)
        self.OutValue.textChanged.connect(self.onOutValueModified)
        self.OutUnit.currentIndexChanged.connect(self.onOutUnitChanged)

        self.onUnitGroupChanged(self.UnitGroup.currentIndex())

        # build inverse map for unit lookup
        self._to_unit = {}
        self._to_group = {}
        for group in UnitConversion.GROUPS:
            for unit in group.units:
                self._to_group[unit] = group
                uns = unit.unit()
                for u in uns:
                    self._to_unit[u.lower()] = unit

    def getUnit(self, unit):
        """
        Get the unit corresponding to 'unit'

        @param unit[str] Textual representation of the unit
        @return [Unit] Unit class corresponding to 'unit'
        """
        try:
            u = self._to_unit[unit.lower()]
            return u
        except:
            return None

    def getUnitGroup(self, unit):
        """
        Get unit group

        @param unit[Unit] Unit class
        @return [UnitGroup] Unit group
        """
        try:
            grp = self._to_group[unit]
            return grp
        except:
            return None

    def updateValue(self, lineedit, value):
        """
        Update text in a QLineEdit without sending signals

        @param lineedit[QLineEdit] Line edit widget
        @param value[float] Value to be set into the line edit
        """
        lineedit.blockSignals(True)
        text = "{:5f}".format(value)
        lineedit.setText(text)
        lineedit.blockSignals(False)

    def convert(self, value, inUnit, outUnit):
        """
        Convert the unit

        @param value[float] Input value
        @param inUnit[Unit] Input unit
        @param outUnit[Unit] Output unit
        @return [float] Converted value
        """
        return outUnit.frm(inUnit.to(value))

    def recompute(self):
        """
        Take the value from the input line edit, convert it and set into output line edit
        """
        inUnit = self.InUnit.itemData(self.InUnit.currentIndex())
        outUnit = self.OutUnit.itemData(self.OutUnit.currentIndex())
        try:
            value = float(self.InValue.text())
            result = self.convert(value, inUnit, outUnit)
            self.updateValue(self.OutValue, result)
        except ValueError:
            pass

    def onSearchEditModified(self, text):
        rx = QtCore.QRegExp("(\S+)\\s+(\S+)\s+[iI][nN]\s+(\S+)")
        if rx.indexIn(text) > -1:
            value = rx.cap(1)
            try:
                val = float(value)
            except ValueError:
                # not a float value
                return

            iu = self.getUnit(rx.cap(2))
            if iu == None:
                # unknown input unit
                return
            ou = self.getUnit(rx.cap(3))
            if ou == None:
                # unknown output unit
                return

            igrp = self.getUnitGroup(iu)
            ogrp = self.getUnitGroup(ou)
            if igrp == ogrp:
                idx = self.UnitGroup.findText(igrp.name())
                self.InUnit.blockSignals(True)
                self.OutUnit.blockSignals(True)

                self.UnitGroup.setCurrentIndex(idx)
                self.InUnit.setCurrentText(iu.name())
                self.OutUnit.setCurrentText(ou.name())

                self.InUnit.blockSignals(False)
                self.OutUnit.blockSignals(False)

                self.InValue.clear()
                self.InValue.setText(value)

    def onUnitGroupChanged(self, index):
        self.InUnit.blockSignals(True)
        self.OutUnit.blockSignals(True)

        group = self.UnitGroup.itemData(index)
        self.InUnit.clear()
        self.OutUnit.clear()
        for unit in group.units:
            self.InUnit.addItem(unit.name(), unit)
            self.OutUnit.addItem(unit.name(), unit)
        self.OutUnit.setCurrentText(self._default_units[group.name()])

        self.InUnit.blockSignals(False)
        self.OutUnit.blockSignals(False)

        if len(self.InValue.text()) > 0:
            self.InValue.setText("1")
            self.recompute()

    def onInValueModified(self, text):
        if len(text) > 0:
            self.recompute()
        else:
            self.OutValue.setText("")

    def onOutValueModified(self, text):
        if len(text) > 0:
            inUnit = self.InUnit.itemData(self.InUnit.currentIndex())
            outUnit = self.OutUnit.itemData(self.OutUnit.currentIndex())
            try:
                value = float(self.OutValue.text())
                result = self.convert(float(text), outUnit, inUnit)
                self.updateValue(self.InValue, result)
            except ValueError:
                pass
        else:
            self.InValue.setText("")

    def onInUnitChanged(self, index):
        self.recompute()

    def onOutUnitChanged(self, index):
        self.recompute()


def main(size = None):
    """
    Run the UnitConverterPlugin alone
    """
    widget = UnitConverterPlugin()
    widget.LayoutMain.setContentsMargins(15, 15, 15, 15)
    widget.show()
    widget.setWindowTitle("Unit Converter")
    return widget

if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    widget = main(size = [400, 100])
    sys.exit(app.exec_())
