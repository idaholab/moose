#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from OtterObject import OtterObject
from FactorySystem import InputParameters

class OtterOutput(OtterObject):

    def validParams():
        params = InputParameters()
        params.addRequiredParam('type', "The type of OtterOutput object.")
        return params
    validParams = staticmethod(validParams)

    def __init__(self, name, params):
        super(OtterOutput, self).__init__(name, params)
        self._system = "Output"
        self._source_warehouse = None

    # register all data source objects here
    def registerDataSourceWarehouse(self, source_warehouse):
        self._source_warehouse = source_warehouse

    # Called to generate the output
    def execute(self):
        raise Exception("override this!")

    # get data by name
    def getDataFrom(self, name):
        if self._source_warehouse is None:
            raise Exception("Internal error. Warehouse not registerd to output object " + self._name)

        if name not in self._source_warehouse:
            raise Exception("Invalid data source '" + name + "' requested by output object " + self._name)

        return self._source_warehouse[name].getData()
