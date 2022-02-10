#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os, sys
from PyQt5 import QtCore, QtWidgets, QtGui
import peacock
import FlowChannelGeometries


class FlowChannelParametersCalculator(QtWidgets.QWidget, peacock.base.Plugin):
    """
    Plugin to compute flow channel parameters
    """

    UNITS_WIDTH = 25

    def __init__(self, **kwargs):
        super(FlowChannelParametersCalculator, self).__init__(**kwargs)

        self.setSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Preferred)

        num_geometris = len(FlowChannelGeometries.GEOMETRIES)
        self.ctlInputs = []
        self.ctlParams = []
        self.btnCalculate = []
        self.lblErrorMessage = []

        self.MainLayout = QtWidgets.QVBoxLayout(self)

        self.ctlFChType = QtWidgets.QComboBox(self)
        self.MainLayout.addWidget(self.ctlFChType)

        self.GeometryLayout = QtWidgets.QStackedLayout()
        for i, g in enumerate(FlowChannelGeometries.GEOMETRIES):
            gname = g.name()
            self.ctlFChType.addItem(gname, i)

            paramsLayout = QtWidgets.QFormLayout()
            paramsLayout.setContentsMargins(5, 0, 5, 0)
            paramsLayout.setLabelAlignment(QtCore.Qt.AlignLeft)
            paramsLayout.setFormAlignment(QtCore.Qt.AlignLeft)
            paramsLayout.setFieldGrowthPolicy(QtWidgets.QFormLayout.ExpandingFieldsGrow)

            self.ctlInputs.append({})
            for p in g.inputs():
                name = p['name']
                unit = p['unit']
                hint = p['hint']

                lblInput = QtWidgets.QLabel(name, self)
                lblInput.setToolTip(hint)

                self.ctlInputs[i][name] = QtWidgets.QLineEdit(self)
                self.ctlInputs[i][name].setToolTip(hint)
                validator = QtGui.QDoubleValidator(self)
                validator.setBottom(0.)
                self.ctlInputs[i][name].setValidator(validator)
                self.ctlInputs[i][name].textChanged.connect(self.onModified)

                lblUnit = QtWidgets.QLabel(unit, self)
                lblUnit.setFixedWidth(self.UNITS_WIDTH)

                hbox = QtWidgets.QHBoxLayout()
                hbox.addWidget(self.ctlInputs[i][name])
                hbox.addWidget(lblUnit)

                paramsLayout.addRow(lblInput, hbox)

            icon_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", 'icons', 'calculator.svg'))
            icon = QtGui.QIcon(icon_path)
            btnCalc = QtWidgets.QPushButton(self)
            btnCalc.clicked.connect(self.onCalculate)
            btnCalc.setIcon(icon)
            btnCalc.setToolTip("Calculate flow channel parameters")
            btnCalc.setAutoDefault(True)
            btnCalc.setMaximumWidth(62)
            paramsLayout.addRow("", btnCalc)
            self.btnCalculate.append(btnCalc)

            # horz line
            ctlLine = QtWidgets.QFrame(self)
            ctlLine.setFrameShape(QtWidgets.QFrame.HLine)
            ctlLine.setFrameShadow(QtWidgets.QFrame.Sunken)
            paramsLayout.addRow(ctlLine)

            self.ctlParams.append({})
            for p in g.outputs():
                name = p['name']
                unit = p['unit']
                hint = p['hint']

                lblParams = QtWidgets.QLabel(name, self)
                lblParams.setToolTip(hint)
                lblParams.setEnabled(False)

                self.ctlParams[i][name] = QtWidgets.QLineEdit(self)
                self.ctlParams[i][name].setReadOnly(True)
                self.ctlParams[i][name].setToolTip(hint)

                lblUnit = QtWidgets.QLabel(unit, self)
                lblUnit.setFixedWidth(self.UNITS_WIDTH)
                lblUnit.setEnabled(False)

                hbox = QtWidgets.QHBoxLayout()
                hbox.addWidget(self.ctlParams[i][name])
                hbox.addWidget(lblUnit)

                paramsLayout.addRow(lblParams, hbox)

            widget = QtWidgets.QWidget()
            widget.setLayout(paramsLayout)
            self.GeometryLayout.addWidget(widget)

            lblErrorMsg = QtWidgets.QLabel(self)
            lblErrorMsg.setStyleSheet("QLabel { color: red; }");
            paramsLayout.addRow(lblErrorMsg)
            self.lblErrorMessage.append(lblErrorMsg)

        self.MainLayout.addLayout(self.GeometryLayout)

        self.CalculateShortcut = QtWidgets.QShortcut(QtGui.QKeySequence("Ctrl+Return"), self)
        self.CalculateShortcut.activated.connect(self.onCtrlReturn)

        self.updateWidgets()

        self.setMainLayoutName('MainLayout')

        self.ctlFChType.model().sort(0)
        self.ctlFChType.activated[int].connect(self.onGeometryTypeChanged)

        self.setup()
        self.store(key='default')

    def _loadPlugin(self):
        """
        Helper for loading plugin state.
        """
        self.load()

    def setup(self):
        super(peacock.base.Plugin, self).setup()
        pass

    def updateWidgets(self):
        idx = self.ctlFChType.currentData()
        enable = True
        for k, input in self.ctlInputs[idx].items():
            if len(input.text()) == 0:
                enable = False
                break
        self.btnCalculate[idx].setEnabled(enable)
        self.CalculateShortcut.setEnabled(enable)

    def onModified(self):
        self.updateWidgets()

    def onCtrlReturn(self):
        idx = self.ctlFChType.currentData()
        self.btnCalculate[idx].animateClick()

    def onCalculate(self):
        self.computeParameters()

    def computeParameters(self):
        """
        Called when the computation of parameters is requested
        """
        g = self.GeometryLayout.currentIndex()
        geom = FlowChannelGeometries.GEOMETRIES[g]

        args = {}
        for i in geom.inputs():
            name = i['name']
            val = float(self.ctlInputs[g][name].displayText())
            args[name] = val

        params = geom.compute(**args)

        if 'error' in params:
            self.lblErrorMessage[g].setText(params['error'])
            for o in geom.outputs():
                name = o['name']
                self.ctlParams[g][name].setText('error')
        else:
            self.lblErrorMessage[g].setText("")
            for o in geom.outputs():
                name = o['name']
                s = "%e" % params[name]
                self.ctlParams[g][name].setText(s)

    def onGeometryTypeChanged(self, index):
        page = self.ctlFChType.itemData(index)
        self.GeometryLayout.setCurrentIndex(page)
        self.updateWidgets()

def main(size=None):
    """
    Run the FlowChannelParametersCalculator alone
    """
    from ModelBuilderPluginManager import ModelBuilderPluginManager
    widget = ModelBuilderPluginManager(plugins=[FlowChannelParametersCalculator])
    widget.MainLayout.setContentsMargins(5, 5, 5, 5)
    widget.show()
    widget.setWindowTitle("Flow Channel Parameter Calculator")
    return widget

if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    widget = main(size=[400,100])
    sys.exit(app.exec_())
