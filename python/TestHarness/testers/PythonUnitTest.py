from RunApp import RunApp
import os

class PythonUnitTest(RunApp):
    @staticmethod
    def validParams():
        params = RunApp.validParams()

        # Input is optional in the base class. Make it required here
        params.addRequiredParam('input', "The python input file to use for this test.")
        params.addParam('buffer', False, "Equivalent to passing -b or --buffer to the unittest.")
        # We don't want to check for any errors on the screen with unit tests
        params['errors'] = []
        return params

    def __init__(self, name, params):
        RunApp.__init__(self, name, params)

    def getCommand(self, options):
        """
        Returns the python command that executes unit tests
        """
        module_name = os.path.splitext(self.specs['input'])[0]
        cmd = 'python -m unittest '
        if self.specs['buffer']:
            cmd += '-b '

        return cmd + module_name + ' ' + ' '.join(self.specs['cli_args'])
