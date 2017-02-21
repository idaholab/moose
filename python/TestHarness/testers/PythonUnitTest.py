from RunApp import RunApp
import os
import util

class PythonUnitTest(RunApp):
    @staticmethod
    def validParams():
        params = RunApp.validParams()

        # Input is optional in the base class. Make it required here
        params.addRequiredParam('input', "The python input file to use for this test.")
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

        return 'python -m unittest ' + module_name + ' ' + ' '.join(self.specs['cli_args'])
