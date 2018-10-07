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

class OtterDataSource(OtterObject):
    @staticmethod
    def validParams():
        params = InputParameters()
        params.addRequiredParam('type', "The type of OtterDataSource object.")
        return params

    def __init__(self, name, params):
        super(OtterDataSource, self).__init__(name, params)
        self._data = None
        self._source_dependencies = {}
        self.system = "DataSource"

    # Called to get the computed the data
    def getData(self):
        if self._data is None:
            raise Exception("Data has not been cmputed yet. Dependency resolution failure")
        return self._data

    # get datasource by name
    def registerDataSourceDependencies(self, names):
        for name in names:
            self._source_dependencies[name] = None

    # replace the Nones with actual objects from teh warehouse
    def applyDataSourceWarehouse(self, source_warehouse):
        for dep in self._source_dependencies:
            self._source_dependencies[dep] = source_warehouse[dep]

    # get data from named source
    def getDataFrom(self, name):
        if name not in self._source_dependencies:
            raise Exception("Trying to get a source that was not registered using registerDataSourceDependencies")

        obj = self._source_dependencies[name]
        if obj is None:
            raise Exception("Failed to fulfill dependency on " + name + " in " + self.name)

        return obj.getData()
