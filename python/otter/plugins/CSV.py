#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from OtterDataSource import OtterDataSource
from FactorySystem import InputParameters

import mooseutils

class CSV(OtterDataSource):
    IS_PLUGIN = True

    def validParams():
        params = OtterDataSource.validParams()
        params.addRequiredParam('file', "The CSV file to read.")
        params.addRequiredParam('x_data', 'X data CSV column name')
        params.addRequiredParam('y_data', 'Y data CSV column name')
        return params
    validParams = staticmethod(validParams)

    def __init__(self, name, params):
        super(CSV, self).__init__(name, params)

    # Called to compute the data
    def execute(self):
        params = self._specs
        # Read data from file
        data = mooseutils.PostprocessorReader(params['file'])
        self._data = (data[params['x_data']].tolist(), data[params['y_data']].tolist())
