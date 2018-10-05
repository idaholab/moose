#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from OtterOutput import OtterOutput
from FactorySystem import InputParameters
from OtterUtils import dataUnion

class WriteCSV(OtterOutput):
    IS_PLUGIN = True

    def validParams():
        params = OtterOutput.validParams()
        params.addRequiredParam('file', "The CSV file to write (each source y is written as a column, interpolated at the union of all x values).")
        params.addRequiredParam('x_data', 'Column name for the x data points')
        params.addRequiredParam('sources', 'DataSources objects to write to a csv file')
        return params
    validParams = staticmethod(validParams)

    def __init__(self, name, params):
        super(WriteCSV, self).__init__(name, params)
        self._file = params['file']
        self._x_name = params['x_data']
        self._sources = params['sources'].split()

    # Called to launch the job
    def execute(self):
        file = open(self._file, "w")

        headers = ','.join([self._x_name] + self._sources)
        datas = [self.getDataFrom(source) for source in self._sources]
        file.write(headers + '\n')

        (data_x, data_ys) = dataUnion(datas)
        for i in range(len(data_x)):
            row = [str(data_x[i])]
            for data_y in data_ys:
                row.append(str(data_y[i]))
            file.write(','.join(row) + '\n')

        file.close()
