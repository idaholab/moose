#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from OtterDataSource import OtterDataSource
from os import access, R_OK
from os.path import isfile

import mooseutils

class CSV(OtterDataSource):
    IS_PLUGIN = True

    @staticmethod
    def validParams():
        params = OtterDataSource.validParams()
        params.addRequiredParam('file', "The CSV file to read.")
        params.addRequiredParam('x_data', 'X data CSV column name')
        params.addRequiredParam('y_data', 'Y data CSV column name')
        return params

    def __init__(self, name, params):
        super(CSV, self).__init__(name, params)
        self._file = params['file']
        self._x_data = params['x_data']
        self._y_data = params['y_data']
        if not isfile(self._file) or not access(self._file, R_OK):
            raise Exception("File %s doesn't exist or isn't readable" % self._file)

    # Called to compute the data
    def execute(self):
        # Read data from file
        data = mooseutils.PostprocessorReader(self._file)
        if self._x_data not in data:
            raise Exception("Column %s not found in '%s'" % (self._x_data, self._file))
        if self._y_data not in data:
            raise Exception("Column %s not found in '%s'" % (self._y_data, self._file))
        self._data = (data[self._x_data].tolist(), data[self._y_data].tolist())
