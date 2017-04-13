from RunApp import RunApp
import os

class PythonUnitTest(RunApp):
    @staticmethod
    def validParams():
        params = RunApp.validParams()

        # Input is optional in the base class. Make it required here
        params.addRequiredParam('input', "The python input file to use for this test.")
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
        module_name = os.path.splitext(self.specs['input'])[0]
        use_buffer = " "
        if self.specs['buffer']:
            use_buffer = " -b "

        if self.specs["separate"]:
            cmd = os.path.join(self.specs['moose_dir'], 'scripts', 'separate_unittests.py') + ' -f ' + module_name + use_buffer
        else:
            cmd = "python -m unittest" + use_buffer + "-v " + module_name

        return cmd  + ' '.join(self.specs['cli_args'])
