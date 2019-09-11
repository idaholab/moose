#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from RunApp import RunApp
import os

class PythonUnitTest(RunApp):
    @staticmethod
    def validParams():
        params = RunApp.validParams()

        # Input is optional in the base class. Make it required here
        params.addRequiredParam('input', "The python input file to use for this test.")
        params.addParam('test_case', "The specific test case to run (Default: All test cases in the module)")
        params.addParam('buffer', False, "Equivalent to passing -b or --buffer to the unittest.")
        params.addParam('separate', False, "Run each test in the file in a separate subprocess")
        # We don't want to check for any errors on the screen with unit tests
        params['errors'] = []
        params['valgrind'] = 'NONE'
        params['recover'] = False
        return params

    def __init__(self, name, params):
        RunApp.__init__(self, name, params)

    def getCommand(self, options):
        """
        Returns the python command that executes unit tests
        """
        test_case = os.path.splitext(self.specs['input'])[0]
        if self.specs.isValid("test_case"):
            test_case += '.' + self.specs['test_case']

        use_buffer = " "
        if self.specs['buffer']:
            use_buffer = " -b "

        if self.specs["separate"]:
            cmd = os.path.join(self.specs['moose_dir'], 'scripts', 'separate_unittests.py') + ' -f ' + test_case + use_buffer
        else:
            cmd = "python -m unittest" + use_buffer + "-v " + test_case

        return cmd  + ' '.join(self.specs['cli_args'])
