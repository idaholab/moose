#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from OtterOutput import OtterOutput
from OtterUtils import dataUnion
import math

class WriteCSV(OtterOutput):
    IS_PLUGIN = True

    @staticmethod
    def validParams():
        params = OtterOutput.validParams()
        params.addRequiredParam('file', "The CSV file to write (each source y is written as a column, interpolated at the union of all x values).")
        params.addRequiredParam('x_name', 'Column name for the x data points')
        params.addRequiredParam('sources', 'DataSources objects to write to a csv file')
        params.addParam('replace_nan', 'Replace "Nan" value with this number')
        return params

    def __init__(self, name, params):
        super(WriteCSV, self).__init__(name, params)
        self._file = params['file']
        self._x_name = params['x_name']
        self._sources = params['sources'].split()
        self._replace_nan = None
        if params.isValid("replace_nan"):
            self._replace_nan = str(params['replace_nan'])

    # Called to launch the job
    def execute(self):
        outfile = open(self._file, "w")

        headers = ','.join([self._x_name] + self._sources)
        datas = [self.getDataFrom(source) for source in self._sources]
        outfile.write(headers + '\n')
        if len(datas) > 1:
            (data_x, data_ys) = dataUnion(datas)
        else:
            (data_x, data_ys) = (datas[0][0], [datas[0][1]])

        replace_nan = self._replace_nan
        for i in range(len(data_x)):
            row = [str(data_x[i])]
            for data_y in data_ys:
                if replace_nan is not None and math.isnan(data_y[i]):
                    row.append(replace_nan)
                else:
                    row.append(str(data_y[i]))
            outfile.write(','.join(row) + '\n')

        outfile.close()
