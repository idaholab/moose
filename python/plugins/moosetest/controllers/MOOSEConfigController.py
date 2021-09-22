#* This file is part of MOOSETOOLS repository
#* https://www.github.com/idaholab/moosetools
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moosetools/blob/main/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import platform
import logging
#import mooseutils
import packaging.version
from moosetools import mooseutils
from moosetools.parameters import InputParameters
from moosetools.moosetest.controllers import AutotoolsConfigController, AutotoolsConfigItem


class MOOSEConfigController(AutotoolsConfigController):
    """
    A base `Controller` to dictate if an object should run based on a C++ configuration.
    """
    AUTO_BUILD = False

    @staticmethod
    def validParams():
        params = AutotoolsConfigController.validParams()
        params.setValue('prefix', 'moose')
        return params

    @staticmethod
    def validObjectParams():
        """
        Return an `parameters.InputParameters` object to be added to a sub-parameter of an object
        with the name given in the "prefix" parameter
        """
        params = AutotoolsConfigController.validObjectParams()
        params.add('ad_mode', allow=('SPARSE', 'NONSPARSE', 'sparse', 'nonsparse'), vtype=str,
                   doc="Limit the test to a specific automatic differentiation derivative type.",
                   user_data=AutotoolsConfigItem('MOOSE_SPARSE_AD', '0', {'0':'NONSPARSE', '1':'SPARSE'}))
        params.add('ad_indexing_type', allow=('GLOBAL', 'LOCAL', 'global', 'local'), vtype=str,
                   doc="Limit the test to global or local automatic differentiation indexing scheme.",
                   user_data=AutotoolsConfigItem('MOOSE_GLOBAL_AD_INDEXING', '0', {'0':'LOCAL', '1':'GLOBAL'}))
        params.add('ad_size', vtype=int,
                   doc="Limit the test to a specific maximum number of DOFs per element for automatic differentiation derivatives.",
                   user_data=AutotoolsConfigItem('MOOSE_AD_MAX_DOFS_PER_ELEM', '50', int))
        params.add('libpng', vtype=bool,
                   doc="Require the existence of the PNG reference library (libpng).",
                   user_data=AutotoolsConfigItem('MOOSE_HAVE_LIBPNG', False, bool))
        return params
