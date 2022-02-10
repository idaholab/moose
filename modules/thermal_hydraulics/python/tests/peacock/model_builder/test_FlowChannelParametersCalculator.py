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
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", 'peacock'))
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", 'peacock', "ModelBuilder"))
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

    def testCircularR(self):
        idx = self._plugin.ctlFChType.itemData(self._plugin.ctlFChType.findText("Circular (radius)"))
        self._plugin.GeometryLayout.setCurrentIndex(idx)

        self._plugin.ctlInputs[idx]['r'].setText('1')
        self._plugin.computeParameters()

        self.assertTrue(self._plugin.ctlParams[idx]['A'].displayText() == '3.141593e+00')
        self.assertTrue(self._plugin.ctlParams[idx]['D_h'].displayText() == '2.000000e+00')
        self.assertTrue(self._plugin.ctlParams[idx]['P_hf'].displayText() == '6.283185e+00')

    def testCircularD(self):
        idx = self._plugin.ctlFChType.itemData(self._plugin.ctlFChType.findText("Circular (diameter)"))
        self._plugin.GeometryLayout.setCurrentIndex(idx)

        self._plugin.ctlInputs[idx]['d'].setText('2')
        self._plugin.computeParameters()

        self.assertTrue(self._plugin.ctlParams[idx]['A'].displayText() == '3.141593e+00')
        self.assertTrue(self._plugin.ctlParams[idx]['D_h'].displayText() == '2.000000e+00')
        self.assertTrue(self._plugin.ctlParams[idx]['P_hf'].displayText() == '6.283185e+00')

    def testSquare(self):
        idx = self._plugin.ctlFChType.itemData(self._plugin.ctlFChType.findText("Square"))
        self._plugin.GeometryLayout.setCurrentIndex(idx)

        self._plugin.ctlInputs[idx]['a'].setText('1')
        self._plugin.computeParameters()

        self.assertTrue(self._plugin.ctlParams[idx]['A'].displayText() == '1.000000e+00')
        self.assertTrue(self._plugin.ctlParams[idx]['D_h'].displayText() == '1.000000e+00')
        self.assertTrue(self._plugin.ctlParams[idx]['P_hf'].displayText() == '4.000000e+00')

    def testRectangular(self):
        idx = self._plugin.ctlFChType.itemData(self._plugin.ctlFChType.findText("Rectangular"))
        self._plugin.GeometryLayout.setCurrentIndex(idx)

        self._plugin.ctlInputs[idx]['a'].setText('2')
        self._plugin.ctlInputs[idx]['b'].setText('3')
        self._plugin.computeParameters()

        self.assertTrue(self._plugin.ctlParams[idx]['A'].displayText() == '6.000000e+00')
        self.assertTrue(self._plugin.ctlParams[idx]['D_h'].displayText() == '2.400000e+00')
        self.assertTrue(self._plugin.ctlParams[idx]['P_hf'].displayText() == '1.000000e+01')

    def testAnnulus(self):
        idx = self._plugin.ctlFChType.itemData(self._plugin.ctlFChType.findText("Annulus"))
        self._plugin.GeometryLayout.setCurrentIndex(idx)

        self._plugin.ctlInputs[idx]['r_in'].setText('1')
        self._plugin.ctlInputs[idx]['r_out'].setText('2')
        self._plugin.computeParameters()

        self.assertTrue(self._plugin.ctlParams[idx]['A'].displayText() == '9.424778e+00')
        self.assertTrue(self._plugin.ctlParams[idx]['D_h'].displayText() == '2.000000e+00')
        self.assertTrue(self._plugin.ctlParams[idx]['P_hf_in'].displayText() == '6.283185e+00')
        self.assertTrue(self._plugin.ctlParams[idx]['P_hf_out'].displayText() == '1.256637e+01')

    def testCoreChannelFuelCylindrical(self):
        idx = self._plugin.ctlFChType.itemData(self._plugin.ctlFChType.findText("Core Channel (cylindrical fuel)"))
        self._plugin.GeometryLayout.setCurrentIndex(idx)

        self._plugin.ctlInputs[idx]['pitch'].setText('2')
        self._plugin.ctlInputs[idx]['r'].setText('0.5')
        self._plugin.computeParameters()

        self.assertTrue(self._plugin.ctlParams[idx]['A'].displayText() == '3.214602e+00')
        self.assertTrue(self._plugin.ctlParams[idx]['D_h'].displayText() == '4.092958e+00')
        self.assertTrue(self._plugin.ctlParams[idx]['P_hf'].displayText() == '3.141593e+00')

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
