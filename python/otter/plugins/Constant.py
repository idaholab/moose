#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from OtterDataSource import OtterDataSource

class Constant(OtterDataSource):
    IS_PLUGIN = True

    @staticmethod
    def validParams():
        params = OtterDataSource.validParams()
        params.addRequiredParam('x_values', 'List of X values')
        params.addRequiredParam('y_values', 'List of Y values')
        return params

    def __init__(self, name, params):
        super(Constant, self).__init__(name, params)
        self._data = (map(float, params['x_values'].split()),
                      map(float, params['y_values'].split()))
        if len(self._data[0]) != len(self._data[1]):
            raise Exception("Provide the same number of x_values as y_values")

    # Called to compute the data
    def execute(self):
        pass
