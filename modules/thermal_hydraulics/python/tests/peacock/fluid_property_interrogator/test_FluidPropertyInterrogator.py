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
import unittest
from PyQt5 import QtWidgets, QtCore, QtGui
import peacock
from FluidPropertyInterrogatorPlugin import main
import time

class TestFluidPropertyInterrogator(unittest.TestCase):
    """
    Testing for FluidPropertyInterrogator.
    """

    #: QApplication: The main App for QT, this must be static to work correctly.
    qapp = QtWidgets.QApplication(sys.argv)

    def setUp(self):
        self._widget = main(size=[600,600])

    def testIGFP(self):
        idx = self._widget.ctlFPType.itemData(self._widget.ctlFPType.findText("IdealGasFluidProperties"))
        self._widget.layoutFPParams.setCurrentIndex(0)

        input_page = self._widget.layoutFPParams.currentWidget()
        # select p-T input page
        input_page.setCurrentIndex(0)

        input_page.setCurrentIndex(0)
        pg = input_page.currentWidget()
        pg.ctlInputs['p'].setText('1e5')
        pg.ctlInputs['T'].setText('300')
        pg.computeProperties()
        # NOTE: cannot properly check the answer, because the slots that read from stdout and stderr
        # and not activated, i.e. nothing gets set into the widgets. In order to test this works, we
        # would need to change the way how properties are computed (i.e. we would not spawn a background
        # process that execute the input file with fluid property interrogator syntax)


if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
