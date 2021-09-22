#* This file is part of MOOSETOOLS repository
#* https://www.github.com/idaholab/moosetools
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moosetools/blob/main/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import shutil
from moosetools import moosetest
from moosetools import mooseutils
from ..runners import MOOSEAppRunner

class DisableController(moosetest.base.Controller):
    """
    A `Controller` for temporarily skipping or removing tests.
    """
    AUTO_BUILD = True

    @staticmethod
    def validParams():
        params = moosetest.base.Controller.validParams()
        params.setValue('prefix', 'disable')
        return params

    @staticmethod
    def validObjectParams():
        """
        Return an `parameters.InputParameters` object to be added to a sub-parameter of an object
        with the name given in the "prefix" parameter
        """
        params = moosetest.base.Controller.validObjectParams()
        params.add('reason', vtype=str, doc="The reason for skipping the test.")
        params.add('method', vtype=str, allow=('SKIP', 'REMOVE'), default='SKIP',
                   doc="The method of disabling the test ('SKIP' or 'REMOVE').")
        return params

    def execute(self, obj, params):
        reason = params.getValue('reason')
        method = params.getValue('method')
        if reason is not None:
            self.skip(reason) if method == 'SKIP' else self.remove(reason)
