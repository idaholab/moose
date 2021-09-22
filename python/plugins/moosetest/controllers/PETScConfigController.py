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

class PETScConfigController(AutotoolsConfigController):
    """
    A base `Controller` to dictate if an object should run based on a C++ configuration.
    """
    AUTO_BUILD = False

    @staticmethod
    def validParams():
        params = AutotoolsConfigController.validParams()
        params.setValue('prefix', 'petsc')
        return params

    @staticmethod
    def validObjectParams():
        """
        Return an `parameters.InputParameters` object to be added to a sub-parameter of an object
        with the name given in the "prefix" parameter
        """
        params = AutotoolsConfigController.validObjectParams()

        # Versions
        params.add('major', vtype=int,
                   doc="Major version for PETSc, as in MAJOR.MINOR.SUBMINOR.",
                   user_data=AutotoolsConfigItem('LIBMESH_DETECTED_PETSC_VERSION_MAJOR', False, int))
        params.add('minor', vtype=int,
                   doc="Minor version for PETSc, as in MAJOR.MINOR.SUBMINOR.",
                   user_data=AutotoolsConfigItem('LIBMESH_DETECTED_PETSC_VERSION_MINOR', 0, int))
        params.add('subminor', vtype=int,
                   doc="Sub-minor version for PETSc, as in MAJOR.MINOR.SUBMINOR.",
                   user_data=AutotoolsConfigItem('LIBMESH_DETECTED_PETSC_VERSION_SUBMINOR', 0, int))
        params.add('version_release', vtype=bool,
                   doc="Require PETSc to be a release version.",
                   user_data=AutotoolsConfigItem('LIBMESH_DETECTED_PETSC_VERSION_RELEASE', True, bool))

        # Build
        params.add('debug', vtype=bool,
                   doc="Require a debug build of PETSc.",
                   user_data=AutotoolsConfigItem('LIBMESH_PETSC_USE_DEBUG', False, bool))

        # Additional, external libraries
        params.add('superlu', vtype=bool,
                   doc="Require the existence of the SuperLU library.",
                   user_data=AutotoolsConfigItem('LIBMESH_PETSC_HAVE_SUPERLU_DIST', False, bool))
        params.add('mumps', vtype=bool,
                   doc="Require the existence of the MUMPS library.",
                   user_data=AutotoolsConfigItem('LIBMESH_PETSC_HAVE_MUMPS', False, bool))
        params.add('strumpack', vtype=bool,
                   doc="Require the existence of the STRUMPACK library.",
                   user_data=AutotoolsConfigItem('LIBMESH_PETSC_HAVE_STRUMPACK', False, bool))
        params.add('parmetis', vtype=bool,
                   doc="Require the existence of the ParMETIS library.",
                   user_data=AutotoolsConfigItem('LIBMESH_PETSC_HAVE_PARMETIS', False, bool))
        params.add('chaco', vtype=bool,
                   doc="Require the existence of the Chaco library.",
                   user_data=AutotoolsConfigItem('LIBMESH_PETSC_HAVE_CHACO', False, bool))
        params.add('party', vtype=bool,
                   doc="Require the existence of the Party library.",
                   user_data=AutotoolsConfigItem('LIBMESH_PETSC_HAVE_PARTY', False, bool))
        params.add('ptscotch', vtype=bool,
                   doc="Require the existence of the PTScotch library.",
                   user_data=AutotoolsConfigItem('LIBMESH_PETSC_HAVE_PTSCOTCH', False, bool))

        # Non-configure based options
        params.add('minimum_version', vtype=str,
                   doc="Minimum version of PETSc, as 'MAJOR.MINOR.SUBMINOR', allowed for this test.")
        params.add('maximum_version', vtype=str,
                   doc="Maximum version of PETSc, as 'MAJOR.MINOR.SUBMINOR', allowed for this test.")
        params.add('version', vtype=str,
                   doc="Specific version of PETSc, as 'MAJOR.MINOR.SUBMINOR', required for this test.")

        return params


    def execute(self, obj, params):
        AutotoolsConfigController.execute(self, obj, params)

        # Determine the version information from the configuration file
        _, major, _ = self.getConfigItem(params, 'major')
        _, minor, _ = self.getConfigItem(params, 'minor')
        _, subminor, _ = self.getConfigItem(params, 'subminor')
        sys_version_text = f"{major}.{minor}.{subminor}"
        sys_version = packaging.version.parse(sys_version_text)
        self.debug("PETSc configured version: {}", sys_version)

        # PETSc min. version
        min_version = params.getValue('minimum_version')
        if (min_version is not None) and (packaging.version.parse(min_version) > sys_version):
            self.skip('PETSc {} > {}', min_version, sys_version)
            self.debug(
                "The configured PETSc version {} is less then the allowed minimum version of {}",
                sys_version, min_version)

        # PETSc max. version
        max_version = params.getValue('maximum_version')
        if (max_version is not None) and (packaging.version.parse(max_version) < sys_version):
            self.skip('PETSc {} < {}', max_version, sys_version)
            self.debug(
                "The configured PETSc version {} is greater then the allowed maximum version of {}",
                sys_version, max_version)

        # PETSc exact version
        the_version = params.getValue('version')
        if (the_version is not None):
            result, expression = AutotoolsConfigController._compareVersions(sys_version_text, the_version)
            if not result:
                self.skip(f'PETSc: not {expression}')
                self.debug(
                    "The configured PETSc version {} is not the specified version of {}",
                sys_version, the_version)
