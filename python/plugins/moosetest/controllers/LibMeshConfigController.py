#* This file is part of MOOSETOOLS repository
#* https://www.github.com/idaholab/moosetools
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moosetools/blob/main/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import re
import subprocess
import packaging.version
from moosetools import mooseutils
from moosetools.parameters import InputParameters
from moosetools.moosetest.controllers import AutotoolsConfigController, AutotoolsConfigItem

class LibMeshConfigController(AutotoolsConfigController):
    """
    A base `Controller` to dictate if an object should run based on a C++ configuration.
    """
    AUTO_BUILD = False

    @staticmethod
    def validParams():
        params = AutotoolsConfigController.validParams()
        params.setValue('prefix', 'libmesh')
        return params

    @staticmethod
    def validObjectParams():
        """
        Return an `parameters.InputParameters` object to be added to a sub-parameter of an object
        with the name given in the "prefix" parameter
        """
        params = AutotoolsConfigController.validObjectParams()

        # libMesh features
        params.add('mesh_mode', vtype=str, allow=('REPLICATED', 'DISTRIBUTED', 'replicated', 'distributed'),
                   doc="Require a specific parallel mesh mode.",
                   user_data=AutotoolsConfigItem('LIBMESH_ENABLE_PARMESH', '0', {'0':'REPLICATED', '1':'DISTRIBUTED'}))
        params.add('dof_id_bytes', vtype=int,
                   doc="Require certain number of bytes for the `dof_id` type.",
                   user_data=AutotoolsConfigItem('LIBMESH_DOF_ID_BYTES', 4, int))
        params.add('unique_id', vtype=bool,
                   doc="The configured parallel mesh mode.",
                   user_data=AutotoolsConfigItem('LIBMESH_ENABLE_UNIQUE_ID', False, bool))
        params.add('methods', vtype=str, array=True,
                   verify=(LibMeshConfigController.verifyMethods, "Unknown method."),
                   doc="Limit to the specified build method(s).")

        # Additional, external libraries
        params.add('dtk', vtype=bool,
                   doc="Require the existence of DTK library.",
                   user_data=AutotoolsConfigItem('LIBMESH_TRILINOS_HAVE_DTK', False, bool))
        params.add('boost', vtype=bool,
                   doc="Require the existence of external BOOST library.",
                   user_data=AutotoolsConfigItem('LIBMESH_HAVE_EXTERNAL_BOOST', False, bool))
        params.add('vtk', vtype=bool,
                   doc="Require the existence of VTK library.",
                   user_data=AutotoolsConfigItem('LIBMESH_HAVE_VTK', False, bool))
        params.add('tecplot', vtype=bool,
                   doc="Require the existence of Tecplot library.",
                   user_data=AutotoolsConfigItem('LIBMESH_HAVE_TECPLOT_API', False, bool))
        params.add('curl', vtype=bool,
                   doc="Require the existence of Curl library.",
                   user_data=AutotoolsConfigItem('LIBMESH_HAVE_CURL', False, bool))
        params.add('fparser_jit', vtype=bool,
                   doc="Require the existence of just-in-time (JIT) fparser library.",
                   user_data=AutotoolsConfigItem('LIBMESH_HAVE_FPARSER_JIT', False, bool))

        # Threading
        params.add('threads', vtype=bool,
                   doc="Require use of threading.",
                   user_data=AutotoolsConfigItem('LIBMESH_USING_THREADS', False, bool))
        params.add('thread_mode', vtype=str, array=True,
                   verify=(LibMeshConfigController.verifyThreadModes, "Unknown thread mode."),
                   doc="Restrict test to specific threading modes.")
        params.add('tbb', vtype=bool,
                   doc="Require use of Intel Threading Building Blocks (TBB) library.",
                   user_data=AutotoolsConfigItem('LIBMESH_HAVE_TBB_API', False, bool))
        params.add('openmp', vtype=bool,
                   doc="Require use of OpenMP threading.",
                   user_data=AutotoolsConfigItem('LIBMESH_USING_OPENMP', False, bool))

        # Compiler
        params.add('compiler', vtype=str, array=True, allow=('CLANG', 'GCC'),
                   doc="Restrict test to specific compiler(s).")
        params.add('_compiler_command', vtype=str, private=True,
                   user_data=AutotoolsConfigItem('LIBMESH_CXX', None, str))

        # Library mode
        params.add('library_mode', vtype=str, allow=("STATIC", "DYNAMIC"),
                   doc="The type of build configuration for the library.")

        return params

    @staticmethod
    def verifyMethods(value):
        """
        Check that the supplied libMesh method(s) are valid.
        """
        allow = set(['opt', 'devel', 'dbg', 'oprof'])
        values = list()
        for v in value:
            match = AutotoolsConfigController.OPERATOR_PREFIX_RE.match(v)
            if match:
                values.append(match.group('value'))
            else:
                values.append(v)
        return all(v.lower() for v in allow)

    @staticmethod
    def verifyThreadModes(value):
        """
        Check that the supplied libMesh method(s) are valid.
        """
        allow = set(['pthreads', 'tbb', 'openmp'])
        values = list()
        for v in value:
            match = AutotoolsConfigController.OPERATOR_PREFIX_RE.match(v)
            if match:
                values.append(match.group('value'))
            else:
                values.append(v)
        return all(v.lower() for v in allow)


    def execute(self, obj, params):
        AutotoolsConfigController.execute(self, obj, params)

        method = os.getenv('METHOD', None)
        methods = params.getValue('methods') or None
        if (methods is not None) and (method is not None) and not all(AutotoolsConfigController._compare(method, m)[0] for m in methods):
            self.info("METHOD={}, but test is limited to '{}'.", method, methods)
            self.skip(f"{method} not in {methods}")

        compiler = self._getCompiler(params)
        compilers = params.getValue('compiler') or None
        if (compilers is not None) and (compiler not in compilers):
            self.info("The compiler is '{}', but this test is limited to '{}'.", compiler, compilers)
            self.skip(f"{compiler} not in {compilers}")

        mode = self._getLibraryMode(params)
        modes = params.getValue('library_mode') or None
        if (modes is not None) and (mode != modes):
            self.info("The library mode is '{}', but this test is limited to '{}'.", mode, modes)
            self.skip(f"{mode} != {modes}")

        mode = self._getThreadMode(params)
        modes = params.getValue('thread_mode') or None
        if (modes is not None) and (mode is not None) and not all(AutotoolsConfigController._compare(mode, m)[0] for m in modes):
            self.info("The thread mode is '{}', but this test is limited to '{}'.", mode, modes)
            self.skip(f"{mode} != {modes}")

    def _getCompiler(self, params):
        cxx_command = self.getConfigItem(params, '_compiler_command')[0].split() + ['-show']
        out = subprocess.run(cxx_command, capture_output=True, text=True)
        if 'clang++' in out.stdout:
            return 'CLANG'
        elif 'g++' in out.stdout:
            return 'GCC'

    def _getLibraryMode(self, params):
        libmesh_config = mooseutils.eval_path(self.getParam('config_files')[0])
        libtool_path = os.path.join(os.path.dirname(libmesh_config), '..', '..', 'contrib', 'bin', 'libtool')
        with open(libtool_path, 'r') as fid:
            content = fid.read()

        match = re.search(r"^build_libtool_libs=(?P<answer>yes|no)$", content, flags=re.MULTILINE)
        if match is None:
            self.error("Unable to determine the type of build ('static' or 'dynamic') from {}", libtool_path)
        else:
            return 'DYNAMIC' if match.group('answer') == 'yes' else 'STATIC'

    def _getThreadMode(self, params):
        use_threads, _, _ = self.getConfigItem(params, 'threads')
        if not use_threads:
            return None

        value, _, _ = self.getConfigItem(params, 'tbb')
        if value:
            return 'TBB'
        value, _, _ = self.getConfigItem(params, 'openmp')
        if value:
            return 'OPENMP'

        return 'PTHREADS'
