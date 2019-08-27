import sys
import unittest
from PyQt5 import QtWidgets, QtCore, QtGui
import peacock
from ModelBuilder.FlowChannelParametersCalculator import main

class TestFlowChannelParametersCalculator(unittest.TestCase):
    """
    Testing for FlowChannelParametersCalculator.
    """

    #: QApplication: The main App for QT, this must be static to work correctly.
    qapp = QtWidgets.QApplication(sys.argv)

    def setUp(self):
        self._widget = main(size=[600,600])
        self._plugin = self._widget.FlowChannelParametersCalculator

    def testCircular(self):
        idx = 0
        self._plugin.GeometryLayout.setCurrentIndex(idx)

        self._plugin.ctlInputs[idx]['r'].setText('1')
        self._plugin.computeParameters()

        self.assertTrue(self._plugin.ctlParams[idx]['A'].displayText() == '3.141593e+00')
        self.assertTrue(self._plugin.ctlParams[idx]['D_h'].displayText() == '2.000000e+00')
        self.assertTrue(self._plugin.ctlParams[idx]['P_hf'].displayText() == '6.283185e+00')

    def testSquare(self):
        idx = 1
        self._plugin.GeometryLayout.setCurrentIndex(idx)

        self._plugin.ctlInputs[idx]['a'].setText('1')
        self._plugin.computeParameters()

        self.assertTrue(self._plugin.ctlParams[idx]['A'].displayText() == '1.000000e+00')
        self.assertTrue(self._plugin.ctlParams[idx]['D_h'].displayText() == '1.000000e+00')
        self.assertTrue(self._plugin.ctlParams[idx]['P_hf'].displayText() == '4.000000e+00')

    def testRectangular(self):
        idx = 4
        self._plugin.GeometryLayout.setCurrentIndex(idx)

        self._plugin.ctlInputs[idx]['a'].setText('2')
        self._plugin.ctlInputs[idx]['b'].setText('3')
        self._plugin.computeParameters()

        self.assertTrue(self._plugin.ctlParams[idx]['A'].displayText() == '6.000000e+00')
        self.assertTrue(self._plugin.ctlParams[idx]['D_h'].displayText() == '2.400000e+00')
        self.assertTrue(self._plugin.ctlParams[idx]['P_hf'].displayText() == '1.000000e+01')

    def testAnnulus(self):
        idx = 2
        self._plugin.GeometryLayout.setCurrentIndex(idx)

        self._plugin.ctlInputs[idx]['r_in'].setText('1')
        self._plugin.ctlInputs[idx]['r_out'].setText('2')
        self._plugin.computeParameters()

        self.assertTrue(self._plugin.ctlParams[idx]['A'].displayText() == '9.424778e+00')
        self.assertTrue(self._plugin.ctlParams[idx]['D_h'].displayText() == '2.000000e+00')
        self.assertTrue(self._plugin.ctlParams[idx]['P_hf_in'].displayText() == '6.283185e+00')
        self.assertTrue(self._plugin.ctlParams[idx]['P_hf_out'].displayText() == '1.256637e+01')

    def testCoreChannelFuelCylindrical(self):
        idx = 3
        self._plugin.GeometryLayout.setCurrentIndex(idx)

        self._plugin.ctlInputs[idx]['pitch'].setText('2')
        self._plugin.ctlInputs[idx]['r'].setText('0.5')
        self._plugin.computeParameters()

        self.assertTrue(self._plugin.ctlParams[idx]['A'].displayText() == '3.214602e+00')
        self.assertTrue(self._plugin.ctlParams[idx]['D_h'].displayText() == '4.092958e+00')
        self.assertTrue(self._plugin.ctlParams[idx]['P_hf'].displayText() == '3.141593e+00')

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
