#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

class OtterObject(object):
    def __init__(self, name, params):
        self._specs = params
        self.name = name

    # Called to launch the job
    def getData(self):
        raise Exception("override this!")

    # get data from source by name
    def getDataFrom(self, name):
        raise Exception("override this!")
