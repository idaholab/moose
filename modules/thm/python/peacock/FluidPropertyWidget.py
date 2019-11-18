from PyQt5 import QtWidgets, QtCore
from PyQt5.QtGui import QPalette, QIcon
from PyQt5.QtCore import Qt, pyqtSignal, pyqtSlot, QProcess, QSize
from PyQt5.QtWidgets import QWidget, QFormLayout, QHBoxLayout, QVBoxLayout, QLabel, QLineEdit, QFrame, QPushButton
import os
import json
from tempfile import mkstemp

class FluidPropertyWidget(QWidget):

    UNITS_WIDTH = 55

    def __init__(self, parent):
        super(FluidPropertyWidget, self).__init__(parent)
        self.parent = parent
        self.modified = False

        self.layoutMain = QVBoxLayout(self)
        self.layoutMain.setContentsMargins(15, 7, 15, 7)

        self.layoutForm = QFormLayout()
        self.layoutForm.setLabelAlignment(Qt.AlignLeft)
        self.layoutForm.setFormAlignment(Qt.AlignLeft)
        self.layoutForm.setFieldGrowthPolicy(QFormLayout.ExpandingFieldsGrow)
        self.layoutForm.setVerticalSpacing(5)

        palette = QPalette()
        palette.setColor(QPalette.Base, Qt.gray);
        palette.setColor(QPalette.Text, Qt.darkGray);

        self.lblInputs = {}
        self.ctlInputs = {}
        self.lblUnit = {}
        self.lblProp = {}
        self.ctlProp = {}

        for i, p in enumerate(self.inputs()):
            name = p['name']
            unit = p['unit']
            hint = p['hint']

            self.lblInputs[name] = QLabel(name, self)
            self.lblInputs[name].setToolTip(hint)

            self.ctlInputs[name] = QLineEdit(self)
            self.ctlInputs[name].setToolTip(hint)
            self.ctlInputs[name].textChanged.connect(self.onModified)

            self.lblUnit[name] = QLabel(unit, self)
            self.lblUnit[name].setFixedWidth(self.UNITS_WIDTH)

            hbox = QHBoxLayout()
            hbox.addWidget(self.ctlInputs[name])
            hbox.addWidget(self.lblUnit[name])

            self.layoutForm.addRow(self.lblInputs[name], hbox)

        self.btnCalculate = QPushButton()
        self.btnCalculate.clicked.connect(self.computeProperties)
        icon_path = os.path.abspath(os.path.join(os.path.dirname(__file__), 'icons', 'calculator.svg'))
        icon = QIcon(icon_path)
        self.btnCalculate.setIcon(icon)
        self.btnCalculate.setToolTip("Calculate fluid properties for given inputs")
        self.btnCalculate.setAutoDefault(True)
        self.btnCalculate.setMaximumWidth(62)
        self.layoutForm.addRow("", self.btnCalculate)

        lbl = QFrame()
        lbl.setMinimumHeight(2)
        self.layoutForm.addRow("", lbl)

        for p in self.outputs():
            name = p['name']
            unit = p['unit']
            hint = p['hint']

            self.lblProp[name] = QLabel(name, self)
            self.lblProp[name].setToolTip(hint)

            palette = self.lblProp[name].palette()
            palette.setCurrentColorGroup(QPalette.Disabled)
            palette.setColorGroup(QPalette.Normal, palette.windowText(),
                palette.button(), palette.light(), palette.dark(), palette.mid(),
                palette.text(), palette.brightText(), palette.base(), palette.window())
            self.lblProp[name].setPalette(palette)

            self.ctlProp[name] = QLineEdit(self)
            self.ctlProp[name].setReadOnly(True)
            self.ctlProp[name].setToolTip(hint)
            self.ctlProp[name].setPalette(palette)

            self.lblUnit[name] = QLabel(unit, self)
            self.lblUnit[name].setFixedWidth(self.UNITS_WIDTH)
            self.lblUnit[name].setPalette(palette)

            hbox = QHBoxLayout()
            hbox.addWidget(self.ctlProp[name])
            hbox.addWidget(self.lblUnit[name])

            self.layoutForm.addRow(self.lblProp[name], hbox)

        self.layoutMain.addLayout(self.layoutForm)

        self.layoutMain.addStretch()

        self.lblError = QLabel("")
        self.lblError.setWordWrap(True)
        self.lblError.setStyleSheet("QLabel { color: red; }");
        self.lblError.setVisible(False)
        self.layoutMain.addWidget(self.lblError)
        self.updateWidgets()

        # process used to execute the binary that will compute the fluid properties
        self.process = QProcess(self)
        self.process.setProcessChannelMode(QProcess.SeparateChannels)
        self.process.started.connect(self._onStarted)
        self.process.readyReadStandardOutput.connect(self._onReadStdOut)
        self.process.readyReadStandardError.connect(self._onReadStdErr)
        self.process.finished.connect(self._onJobFinished)
        self.process.error.connect(self._onError)

        # create a temporary file which is used to generate the input file
        self.fd, self.input_file_name = mkstemp()

    def updateWidgets(self):
        enable = True
        for input in self.inputs():
            if len(self.ctlInputs[input['name']].text()) == 0:
                enable = False
                break
        self.btnCalculate.setEnabled(enable)

        title = "Fluid Property Interrogator"
        if self.modified:
            title += " *"
        self.parent.setWindowTitle(title)

    def onModified(self):
        self.setModified(True)
        self.updateWidgets()

    def keyPressEvent(self, event):
        if event.key() == Qt.Key_Return:
            if self.btnCalculate.isEnabled():
                self.computeProperties()

    def setExecutablePath(self, exe_path):
        self.exe_path = exe_path

    def computeProperties(self):
        """
        Called when the computation of properties is requested
        """

        self.buildInputFile(self.input_file_name)

        args = [ '-i', self.input_file_name, '--no-color']
        self.process.start(self.exe_path, args)
        self.process.waitForStarted()
        self.setModified(False)
        self.updateWidgets()

    def setModified(self, modified):
        self.modified = modified

    def buildInputFile(self, file_name):
        """
        Write the input file into a file

        Inputs:
            file_name[str]: File name which we write into
        """

        with open(file_name, "w+") as f:
            f.write("[FluidPropertiesInterrogator]\n")
            f.write("  fp = fp\n")
            f.write("  json = true\n")
            for p in self.inputs():
                name = p['name']
                f.write("  {} = {}\n".format(name, self.ctlInputs[name].text()))

            f.write("[]\n")
            f.write("[Modules]\n")
            f.write("  [./FluidProperties]\n")
            f.write("    {}\n".format(self.parent.fluidPropertyInputFileBlock()))
            f.write("  [../]\n")
            f.write("[]\n")

    @pyqtSlot()
    def _onStarted(self):
        self.json_str = ""
        self.json_data_on = False
        self.error_str = ""
        self.error_data_on = False

    @pyqtSlot()
    def _onReadStdOut(self):
        # store only the JSON data
        self.process.setReadChannel(QProcess.StandardOutput)
        while self.process.canReadLine():
            line = self.process.readLine().data().decode("utf-8").rstrip()
            if line == '**START JSON DATA**':
                self.json_data_on = True
            elif line == '**END JSON DATA**':
                self.json_data_on = False
            elif self.json_data_on:
                self.json_str += line

    @pyqtSlot()
    def _onReadStdErr(self):
        self.process.setReadChannel(QProcess.Exception)
        while self.process.canReadLine():
            line = self.process.readLine().data().decode("utf-8").rstrip()
            if line == '*** ERROR ***':
                self.error_data_on = True
            elif line[:13] == 'Stack frames:':
                self.error_data_on = False
            elif self.error_data_on:
                self.error_str += line

    @pyqtSlot(int, QProcess.ExitStatus)
    def _onJobFinished(self, code, status):
        if code == 0:
            try:
                j = json.loads(self.json_str)
                # enter the data into the controls
                for p in self.outputs():
                    name = p['name']
                    val = str(j[self.jsonSectionName()][name])
                    self.ctlProp[name].setText(val)
            except:
                # this would happen if people used MOOSE that does not support
                # printing fluid properties in JSON format
                pass
            self.lblError.setVisible(False)
        else:
            self.lblError.setText(self.error_str)
            self.lblError.setVisible(True)

    @pyqtSlot(QProcess.ProcessError)
    def _onError(self, err):
        print("error:", err)


class FluidPropertyWidget1PhasePT(FluidPropertyWidget):

    def inputs(self):
        return [
            { 'name': "p", 'unit': "Pa", 'hint': "Pressure" },
            { 'name': "T", 'unit': "K", 'hint': "Temperature" }
        ]

    def outputs(self):
        return [
            { 'name': "rho", 'unit': "kg/m^3", 'hint': "Density" },
            { 'name': "v", 'unit': "m^3/kg", 'hint': "Specific volume" },
            { 'name': "e", 'unit': "J/kg", 'hint': "Specific internal energy" },
            { 'name': "h", 'unit': "J/kg", 'hint': "Specific enthalpy" },
            { 'name': "s", 'unit': "J/kg", 'hint': "Specific entropy" },
            { 'name': "c", 'unit': "m/s", 'hint': "Sound speed" },
            { 'name': "mu", 'unit': "Pa-s", 'hint': "Dynamic viscosity" },
            { 'name': "cp", 'unit': "J/(kg-K)", 'hint': "Specific heat at constant pressure" },
            { 'name': "cv", 'unit': "J/(kg-K)", 'hint': "Specific heat at constant volume" },
            { 'name': "k", 'unit': "W/(m-K)", 'hint': "Thermal conductivity" },
            { 'name': "beta", 'unit': "1/K", 'hint': "Volumetric expansion coefficient" }
        ]

    def jsonSectionName(self):
        return 'static'

class FluidPropertyWidget1PhaseRhoE(FluidPropertyWidget):

    def inputs(self):
        return [
            { 'name': "rho", 'unit': "kg/m^3", 'hint': "Density" },
            { 'name': "e", 'unit': "J/kg", 'hint': "Specific internal energy" },
        ]

    def outputs(self):
        return [
            { 'name': "p", 'unit': "Pa", 'hint': "Pressure" },
            { 'name': "T", 'unit': "K", 'hint': "Temperature" },
            { 'name': "v", 'unit': "m^3/kg", 'hint': "Specific volume" },
            { 'name': "h", 'unit': "J/kg", 'hint': "Specific enthalpy" },
            { 'name': "s", 'unit': "J/kg", 'hint': "Specific entropy" },
            { 'name': "c", 'unit': "m/s", 'hint': "Sound speed" },
            { 'name': "mu", 'unit': "Pa-s", 'hint': "Dynamic viscosity" },
            { 'name': "cp", 'unit': "J/(kg-K)", 'hint': "Specific heat at constant pressure" },
            { 'name': "cv", 'unit': "J/(kg-K)", 'hint': "Specific heat at constant volume" },
            { 'name': "k", 'unit': "W/(m-K)", 'hint': "Thermal conductivity" },
            { 'name': "beta", 'unit': "1/K", 'hint': "Volumetric expansion coefficient" }
        ]

    def jsonSectionName(self):
        return 'static'

class FluidPropertyWidget1PhaseRhoP(FluidPropertyWidget):

    def inputs(self):
        return [
            { 'name': "rho", 'unit': "kg/m^3", 'hint': "Density" },
            { 'name': "p", 'unit': "Pa", 'hint': "Pressure" },
        ]

    def outputs(self):
        return [
            { 'name': "T", 'unit': "K", 'hint': "Temperature" },
            { 'name': "e", 'unit': "J/kg", 'hint': "Specific internal energy" },
            { 'name': "v", 'unit': "m^3/kg", 'hint': "Specific volume" },
            { 'name': "h", 'unit': "J/kg", 'hint': "Specific enthalpy" },
            { 'name': "s", 'unit': "J/kg", 'hint': "Specific entropy" },
            { 'name': "c", 'unit': "m/s", 'hint': "Sound speed" },
            { 'name': "mu", 'unit': "Pa-s", 'hint': "Dynamic viscosity" },
            { 'name': "cp", 'unit': "J/(kg-K)", 'hint': "Specific heat at constant pressure" },
            { 'name': "cv", 'unit': "J/(kg-K)", 'hint': "Specific heat at constant volume" },
            { 'name': "k", 'unit': "W/(m-K)", 'hint': "Thermal conductivity" },
            { 'name': "beta", 'unit': "1/K", 'hint': "Volumetric expansion coefficient" }
        ]

    def jsonSectionName(self):
        return 'static'


class FluidPropertyWidget2PhaseP(FluidPropertyWidget):

    def inputs(self):
        return [
            { 'name': "p", 'unit': "Pa", 'hint': "Pressure" },
        ]

    def outputs(self):
        return [
            { 'name': "p_critical", 'unit': "Pa", 'hint': "Critical pressure" },
            { 'name': "T_sat", 'unit': "Pa", 'hint': "Saturation temperature" },
            { 'name': "h_lat", 'unit': "J/kg", 'hint': "Latent heat of vaporization" },
        ]

    def jsonSectionName(self):
        return '2-phase'

class FluidPropertyWidget2PhaseT(FluidPropertyWidget):

    def inputs(self):
        return [
            { 'name': "T", 'unit': "K", 'hint': "Temperature" },
        ]

    def outputs(self):
        return [
            { 'name': "p_critical", 'unit': "Pa", 'hint': "Critical pressure" },
            { 'name': "p_sat", 'unit': "Pa", 'hint': "Saturation pressure" },
            { 'name': "h_lat", 'unit': "J/kg", 'hint': "Latent heat of vaporization" },
            { 'name': "sigma", 'unit': "N/m", 'hint': "Surface tension" },
        ]

    def jsonSectionName(self):
        return '2-phase'
