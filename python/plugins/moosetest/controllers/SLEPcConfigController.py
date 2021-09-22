#* This file is part of MOOSETOOLS repository
#* https://www.github.com/idaholab/moosetools
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moosetools/blob/main/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import packaging.version
from moosetools import mooseutils
from moosetools.parameters import InputParameters
from moosetools.moosetest.controllers import AutotoolsConfigController, AutotoolsConfigItem

class SLEPcConfigController(AutotoolsConfigController):
    """
    A base `Controller` to dictate if an object should run based on a C++ configuration.
    """
    @staticmethod
    def validParams():
        params = AutotoolsConfigController.validParams()

        # TODO: RunApp has a 'slepc' True|False option, which need to be removed in favor of the
        #       these sub-options (add slepc_required) to server replace 'slepc' param
        params.setValue('prefix', 'theslepc') # 'slepc' conflicts with 'slepc' param in legacy...
        return params

    @staticmethod
    def validObjectParams():
        """
        Return an `parameters.InputParameters` object to be added to a sub-parameter of an object
        with the name given in the "prefix" parameter
        """
        params = AutotoolsConfigController.validObjectParams()

        params.add('required', vtype=bool,
                   doc="Require the existence of SLEPc library.",
                   user_data=AutotoolsConfigItem('LIBMESH_HAVE_SLEPC', False, bool))
        params.add('major', vtype=int,
                   doc="Major version for SLEPc, as in MAJOR.MINOR.SUBMINOR.",
                   user_data=AutotoolsConfigItem('LIBMESH_DETECTED_SLEPC_VERSION_MAJOR', 0, int))
        params.add('minor', vtype=int,
                   doc="Minor version for SLEPc, as in MAJOR.MINOR.SUBMINOR.",
                   user_data=AutotoolsConfigItem('LIBMESH_DETECTED_SLEPC_VERSION_MINOR', 0, int))
        params.add('subminor', vtype=int,
                   doc="Sub-minor version for SLEPc, as in MAJOR.MINOR.SUBMINOR.",
                   user_data=AutotoolsConfigItem('LIBMESH_DETECTED_SLEPC_VERSION_SUBMINOR', 0, int))

        params.add('minimum_version', vtype=str,
                   doc="Minimum version of SLEPc, as 'MAJOR.MINOR.SUBMINOR', allowed for this test.")
        params.add('maximum_version', vtype=str,
                   doc="Maximum version of SLEPc, as 'MAJOR.MINOR.SUBMINOR', allowed for this test.")
        params.add('version', vtype=str,
                   doc="Specific version of SLEPc, as 'MAJOR.MINOR.SUBMINOR', required for this test.")

        return params


    def execute(self, obj, params):
        AutotoolsConfigController.execute(self, obj, params)

        # Determine the version information from the configuration file
        _, major, _ = self.getConfigItem(params, 'major')
        _, minor, _ = self.getConfigItem(params, 'minor')
        _, subminor, _ = self.getConfigItem(params, 'subminor')
        sys_version_text = f"{major}.{minor}.{subminor}"
        sys_version = packaging.version.parse(sys_version_text)
        self.debug("SLEPc configured version: {}", sys_version)

        # SLEPc min. version
        min_version = params.getValue('minimum_version')
        if (min_version is not None) and (packaging.version.parse(min_version) > sys_version):
            self.skip('SLEPc {} > {}', min_version, sys_version)
            self.debug(
                "The configured SLEPc version {} is less then the allowed minimum version of {}",
                sys_version, min_version)

        # SLEPc max. version
        max_version = params.getValue('maximum_version')
        if (max_version is not None) and (packaging.version.parse(max_version) < sys_version):
            self.skip('SLEPc {} < {}', max_version, sys_version)
            self.debug(
                "The configured SLEPc version {} is greater then the allowed maximum version of {}",
                sys_version, max_version)

        # SLEPc exact version
        the_version = params.getValue('version')
        if (the_version is not None):
            result, expression = AutotoolsConfigController._compareVersions(sys_version_text, the_version)
            if not result:
                self.skip(f'SLEPc: not {expression}')
                self.debug(
                    "The configured SLEPc version {} is not the specified version of {}",
                sys_version, the_version)
