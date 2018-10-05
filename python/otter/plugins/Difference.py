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
from OtterUtils import dataUnion
import mooseutils

class Difference(OtterDataSource):
    IS_PLUGIN = True

    def validParams():
        params = OtterDataSource.validParams()
        params.addRequiredParam('source1', 'The base data source')
        params.addRequiredParam('source2', 'The source to subtract from the base source')
        return params
    validParams = staticmethod(validParams)

    def __init__(self, name, params):
        super(Difference, self).__init__(name, params)

        # register dependencies
        self._source1 = params['source1']
        self._source2 = params['source2']
        self.registerDataSourceDependencies([self._source1, self._source2])

    # Called to compute the data
    def execute(self):
        data1 = self.getDataFrom(self._source1)
        data2 = self.getDataFrom(self._source2)
        (data_x, [data_y1, data_y2]) = dataUnion([data1, data2])

        self._data = (data_x,[])

        # compute difference and check ordinate
        for i in range(len(data_x)):
            self._data[1].append(data_y1[i] - data_y2[i])
