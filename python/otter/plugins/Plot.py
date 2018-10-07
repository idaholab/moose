#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from OtterOutput import OtterOutput
import matplotlib.pyplot as plt

class Plot(OtterOutput):
    IS_PLUGIN = True

    @staticmethod
    def validParams():
        params = OtterOutput.validParams()
        params.addRequiredParam('file', "The Plot file to write (file extension determines the format).")
        params.addRequiredParam('sources', 'DataSources objects to plot')
        params.addParam('x_label', 'X-Axis label')
        params.addParam('y_label', 'Y-Axis label')
        return params

    def __init__(self, name, params):
        super(Plot, self).__init__(name, params)

    # Called to launch the job
    def execute(self):
        plt.figure(figsize=(6.0, 4.0))
        params = self._specs

        for source in params['sources'].split():
            data = self.getDataFrom(source)
            plt.plot(data[0], data[1], label=source)

        plt.legend()

        if params.isValid('x_label'):
            plt.xlabel(params['x_label'])
        if params.isValid('y_label'):
            plt.ylabel(params['y_label'])

        plt.savefig(params['file'], bbox_inches='tight', pad_inches=0)
