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

import chigger

class ExodusGlobal(OtterDataSource):
    IS_PLUGIN = True

    def validParams():
        params = OtterDataSource.validParams()
        params.addRequiredParam('file', "The Exodus file to read.")
        params.addParam('x_data', 'X data from exodus to plot (if not specified time is used)')
        params.addParam('y_data', 'Y data from exodus to plot (if not specified time is used)')
        return params
    validParams = staticmethod(validParams)

    def __init__(self, name, params):
        super(ExodusGlobal, self).__init__(name, params)

        self._globals = [None, None]
        for i, p in enumerate(['x_data', 'y_data']):
            if params.isValid(p):
                self._globals[i] = params[p]


    # Called to compute the data
    def execute(self):
        params = self._specs

        # Read data from file
        reader = chigger.exodus.ExodusReader(params['file'])
        times = reader.getTimes()
        self._data = ([], [])
        for i, t in enumerate(times):
            reader.update(timestep=i)

            # append either specified global or time
            for j in (0, 1):
                if self._globals[j] is None:
                    self._data[j].append(t)
                else:
                    self._data[j].append(reader.getGlobalData(self._globals[j]))
