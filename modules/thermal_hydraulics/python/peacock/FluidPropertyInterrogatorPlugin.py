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
from PyQt5.QtGui import QStandardItem, QStandardItemModel, QIcon
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QSizePolicy, QVBoxLayout, QFormLayout, QStackedLayout, QWidget, QComboBox, QTabWidget
from FluidPropertyWidget import *
import peacock
from peacock.Input.ExecutableInfo import ExecutableInfo


class FluidPropertyInterrogatorPlugin(QtWidgets.QWidget):
    """
    Fluid property interrogator GUI
    """

    SINGLE_PHASE = 1
    TWO_PHASE = 2
    TWO_PHASE_NCG = 3

    def __init__(self, **kwargs):
        super(FluidPropertyInterrogatorPlugin, self).__init__(**kwargs)

        self.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Preferred)

        self.layoutMain = QVBoxLayout(self)

        self.ctlFPType = QComboBox(self)
        self.layoutMain.addWidget(self.ctlFPType)

        self.layoutFPParams = QStackedLayout()
        self.ctlSinglePhaseWidget = self.setupSinglePhaseUI(self.layoutFPParams)
        self.ctlTwoPhaseWidget = self.setupTwoPhaseUI(self.layoutFPParams)
        self.layoutFPParams.addWidget(self.ctlSinglePhaseWidget)
        self.layoutFPParams.addWidget(self.ctlTwoPhaseWidget)
        self.layoutMain.addLayout(self.layoutFPParams)

        self.ctlFPType.currentIndexChanged.connect(self.onFluidPropertyChanged)

    def setupSinglePhaseUI(self, layout):
        widget = QTabWidget(self)
        widget.addTab(FluidPropertyWidget1PhasePT(self), "p, T")
        widget.addTab(FluidPropertyWidget1PhaseRhoE(self), "rho, e")
        widget.addTab(FluidPropertyWidget1PhaseRhoP(self), "rho, p")
        return widget

    def setupTwoPhaseUI(self, layout):
        widget = QTabWidget(self)
        widget.addTab(FluidPropertyWidget2PhaseP(self), "p")
        widget.addTab(FluidPropertyWidget2PhaseT(self), "T")
        return widget

    def fillFluidProperties(self):
        """
        Fill the combo box with single phase fluid property classes
        """

        fp_classes = [
            {
                'name': 'Single phase',
                'classes': [],
                'data': self.SINGLE_PHASE
            },
            {
                'name': 'Two phase',
                'classes': [],
                'data': self.TWO_PHASE
            }
        ]

        json_fps = self.exe_info.json_data.json_data["blocks"]['Modules']['subblocks']['FluidProperties']['star']['subblock_types']

        for class_name, vals in json_fps.items():
            params = vals['parameters']
            if 'fp_type' in params:
                fp_type = params['fp_type']['default']
                if fp_type == 'single-phase-fp':
                    fp_classes[0]['classes'].append(class_name)
                elif fp_type == 'two-phase-fp':
                    fp_classes[1]['classes'].append(class_name)

        for fpc in fp_classes:
            fpc['classes'].sort()

        sep_indices = []
        model = QStandardItemModel()
        for fpc in fp_classes:
            parent = QStandardItem(fpc['name'])
            parent.setEnabled(False)
            model.appendRow(parent)
            for c in fpc['classes']:
                item = QStandardItem(c)
                item.setData(fpc['data'])
                model.appendRow(item)
            sep_indices.append(model.rowCount())

        self.ctlFPType.setModel(model)
        for idx in sep_indices[:-1]:
            self.ctlFPType.insertSeparator(idx)

    def onFluidPropertyChanged(self, index):
        fp_type = self.ctlFPType.model().item(index).data()
        if fp_type == self.SINGLE_PHASE:
            self.layoutFPParams.setCurrentWidget(self.ctlSinglePhaseWidget)
            self.ctlSinglePhaseWidget.currentWidget().updateWidgets()
        elif fp_type == self.TWO_PHASE:
            self.layoutFPParams.setCurrentWidget(self.ctlTwoPhaseWidget)
            self.ctlTwoPhaseWidget.currentWidget().updateWidgets()
        else:
            raise SystemExit("Unknown fp_type")

    def fluidPropertyInputFileBlock(self):
        str = ""
        str += "[./fp]\n"
        str += "type = {}\n".format(self.ctlFPType.currentText())
        str += "[../]\n"
        return str

    def setExecutablePath(self, exe_path):
        self.exe_path = exe_path
        self.exe_info = ExecutableInfo()
        self.exe_info.setPath(self.exe_path)
        self.fillFluidProperties()

        for tab in range(self.ctlSinglePhaseWidget.count()):
            self.ctlSinglePhaseWidget.widget(tab).setExecutablePath(exe_path)
        for tab in range(self.ctlTwoPhaseWidget.count()):
            self.ctlTwoPhaseWidget.widget(tab).setExecutablePath(exe_path)

def main(size=None):
    """
    Run the FluidPropertyInterrogatorPlugin alone
    """
    from peacock.utils import ExeFinder

    widget = FluidPropertyInterrogatorPlugin()
    exe_path = ExeFinder.getExecutablePath(None, start_dir = os.getcwd())
    if exe_path:
        widget.setExecutablePath(exe_path)
        widget.layoutMain.setContentsMargins(15, 15, 15, 15)
        widget.show()
        widget.setWindowTitle("Fluid Property Interrogator")
        return widget
    else:
        raise SystemExit("No executable found")

if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    widget = main(size=[400,100])
    sys.exit(app.exec_())
