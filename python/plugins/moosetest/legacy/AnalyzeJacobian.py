import os
from moosetools import mooseutils
from moosetools import moosetest
from .RunApp import RunApp

class AnalyzeJacobian(RunApp):
    """
    Run python jacobiananalyzer.py script for performing Jacobian checks.

    Direct replacement for legacy TestHarness AnalyzeJacobian Tester object.
    """

    @staticmethod
    def validParams():
        params = RunApp.validParams()
        params.add('resize_mesh', vtype=bool, default=False, doc="Enable re-sizing of the input mesh using the 'mesh_size' parameter.")
        params.add('off_diagonal', default=True, doc="Test the off-diagonal Jacobian entries.")
        params.add('mesh_size', default=1, doc="Value supplied to '-s' option for re-sizing the input mesh.")
        return params

    def execute(self):

        # Locate the script to run
        script = os.path.join(os.path.dirname(__file__), '..', '..', '..', 'jacobiandebug', 'analyzejacobian.py')
        if not os.path.isfile(script):
            self.error("Analyze Jacobian script not found: {}", script)
            return 1

        # Locate the MOOSE application executable
        exe = mooseutils.find_moose_executable_recursive()
        if exe is None:
            self.critical("Unable to locate MOOSE application executable starting in '{}'.", os.getcwd())
            return 1

        # Build the command to run
        command = [script, '-e', exe, '-i']
        command += [infile.strip() for infile in self.getParam('input')]
        if self.getParam('resize_mesh'):
            command += ['-r', '-s', str(self.getParam('mesh_size'))]

        if not self.getParam('off_diagonal'):
            command += ['-D']

        if self.isParamValid('cli_args'):
            command += ['--cli-args']
            command += mooseutils.separate_args(self.getParam('cli_args'))

        self.parameters().setValue('command', tuple(command))
        return moosetest.runners.ExecuteCommand.execute(self) # by-pass MOOSE stuff
