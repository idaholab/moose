import os, sys
from PyQt5 import QtCore, QtWidgets, QtGui
from PyQt5.QtCore import Qt, pyqtSignal, pyqtSlot, QProcess
from PyQt5.QtWidgets import QSizePolicy
import peacock
import FlowChannelGeometries


class FlowChannelParametersCalculator(QtWidgets.QWidget, peacock.base.Plugin):
    """
    Plugin to compute flow channel parameters
    """

    UNITS_WIDTH = 25

    def __init__(self, **kwargs):
        super(FlowChannelParametersCalculator, self).__init__(**kwargs)

        self.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Preferred)

        num_geometris = len(FlowChannelGeometries.GEOMETRIES)
        self.ctlInputs = []
        self.ctlParams = []

        self.MainLayout = QtWidgets.QVBoxLayout(self)

        self.ctlFChType = QtWidgets.QComboBox(self)
        self.MainLayout.addWidget(self.ctlFChType)

        self.GeometryLayout = QtWidgets.QStackedLayout()
        for i, g in enumerate(FlowChannelGeometries.GEOMETRIES):
            gname = g.name()
            self.ctlFChType.addItem(gname)

            paramsLayout = QtWidgets.QFormLayout()
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
                self.ctlInputs[i][name].returnPressed.connect(self.computeParameters)

                lblUnit = QtWidgets.QLabel(unit, self)
                lblUnit.setFixedWidth(self.UNITS_WIDTH)

                hbox = QtWidgets.QHBoxLayout()
                hbox.addWidget(self.ctlInputs[i][name])
                hbox.addWidget(lblUnit)

                paramsLayout.addRow(lblInput, hbox)

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

                palette = lblParams.palette()
                palette.setCurrentColorGroup(QtGui.QPalette.Disabled)
                palette.setColorGroup(QtGui.QPalette.Normal, palette.windowText(),
                    palette.button(), palette.light(), palette.dark(), palette.mid(),
                    palette.text(), palette.brightText(), palette.base(), palette.window())
                lblParams.setPalette(palette)

                self.ctlParams[i][name] = QtWidgets.QLineEdit(self)
                self.ctlParams[i][name].setReadOnly(True)
                self.ctlParams[i][name].setToolTip(hint)
                self.ctlParams[i][name].setFocusPolicy(QtCore.Qt.NoFocus)

                lblUnit = QtWidgets.QLabel(unit, self)
                lblUnit.setFixedWidth(self.UNITS_WIDTH)
                lblUnit.setPalette(palette)

                hbox = QtWidgets.QHBoxLayout()
                hbox.addWidget(self.ctlParams[i][name])
                hbox.addWidget(lblUnit)

                paramsLayout.addRow(lblParams, hbox)

            widget = QtWidgets.QWidget()
            widget.setLayout(paramsLayout)
            self.GeometryLayout.addWidget(widget)

        self.MainLayout.addLayout(self.GeometryLayout)

        self.setMainLayoutName('MainLayout')

        self.ctlFChType.activated[int].connect(self.GeometryLayout.setCurrentIndex)

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

        for o in geom.outputs():
            name = o['name']
            s = "%e" % params[name]
            self.ctlParams[g][name].setText(s)


def main(size=None):
    """
    Run the FlowChannelParametersCalculator alone
    """
    from ModelBuilderPluginManager import ModelBuilderPluginManager
    widget = ModelBuilderPluginManager(plugins=[FlowChannelParametersCalculator])
    widget.show()
    widget.setWindowTitle("Flow Channel Parameter Calculator")
    return widget

if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    widget = main(size=[400,100])
    sys.exit(app.exec_())
