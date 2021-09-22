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

class ValgrindController(moosetest.base.Controller):
    """
    A `Controller` for running with Valgrind.
    """
    AUTO_BUILD = False
    OBJECT_TYPES = (MOOSEAppRunner,)

    @staticmethod
    def validParams():
        params = moosetest.base.Controller.validParams()
        params.setValue('prefix', 'mem')


        # TODO: Use TagController
        params.add('mode', vtype=str, allow=('NORMAL', 'HEAVY'),
                   doc="Enable the use of Valgrind for memory checking with the given mode of operation.")
        params.add('suppressions', doc="Suppression file(s) to use when running with Valgrind.")
        return params

    @staticmethod
    def validObjectParams():
        """
        Return an `parameters.InputParameters` object to be added to a sub-parameter of an object
        with the name given in the "prefix" parameter
        """
        params = moosetest.base.Controller.validObjectParams()
        params.add('mode', vtype=str, allow=('NONE', 'NORMAL', 'HEAVY', 'none', 'normal', 'heavy'),
                   doc=("Restrict the test to a specific mode of operation: 'NONE' disables use of "
                        "Valgrind and skips the test."))
        return params

    @staticmethod
    def validCommandLineArguments(parser, params):

        parser.add_argument('--valgrind', type=str, choices=('NORMAL', 'HEAVY'),
                            help="Execute with memory checking using Valgrind.")
        return parser

    def _setup(self, args):
        moosetest.base.Controller._setup(self, args)
        self.parameters().setValue('mode', args.valgrind)

    def execute(self, obj, params):

        # Return if Controller "mode" parameters is not set
        mode = self.getParam('mode')
        if mode is None:
            return

        # Check that object is configured to run in the current mode
        obj_mode = params.getValue('mode')
        if (obj_mode is not None) and obj_mode != mode:
            msg = "The Valgrind mode for the test ({}) does not match the mode being executed ({})."
            self.debug(msg, obj_mode, mode)
            self.skip('valgrind: {} != {}', obj_mode, mode)
            return

        # Check for command
        if shutil.which('valgrind') is None:
            msg = "The 'valgrind' executable does not exist."
            self.error(msg)
            return

        # Valgrind command
        cmd = ['valgrind', '--leak-check=full', '--tool=memcheck', '--dsymutil=yes',
               '--track-origins=yes', '--demangle=yes', '-v']
        supp = self.getParam('suppressions')
        if supp is not None:
            cmd += ['--suppressions'] + [mooseutils.eval_path(p) for p in supp]

        # Update the Runner object with the new command
        params.setValue('command', cmd + list(params.getParam('command') or tuple()))
