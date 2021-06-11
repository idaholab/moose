import os
import subprocess

from moosetools.moosetest import runners
from moosetools.mooseutils import find_moose_executable_recursive

from . import helpers

class MooseAppRunner(runners.RunCommand):

    @staticmethod
    def validParams():
        params = runners.RunCommand.validParams()
        #params.add('executable', vtype=str, doc="The executable to run, by default this is located automatically.")
        params.setRequired('command', False)

        params.add('input', vtype=str, required=True,
                   doc="The input file (*.i) to utilize for running application. The file should be defined relative to the HIT test specification or the current working directory if the object is not being instantiated with a test specification.")


        params.add('scale_refine')
        params.add('cli_args')
        params.add('prereq')

        params.add('recover')
        params.add('petsc_version')


        # TODO: Create SQAController, then set the sqa_design, etc. parameters using these, perhaps
        #       a mixin object in MOOSE can do this sort of thig
        params.add('design')
        params.add('requirement')
        params.add('issues')

        return params

    def execute(self):
        #kwargs = dict()
        #kwargs['capture_output'] = False # use sys.stdout/sys.stderr, which is captured by TestCase
        #kwargs['text'] = True # encode output to UTF-8
        #kwargs['check'] = self.getParam('allow_exception')
        #kwargs['timeout'] = self.getParam('timeout')

        # Command list to supply base RunCommand
        command = list()

        # Determine working location
        base_dir = helpers.get_file_base(self)

        # Locate MOOSE application executable
        exe = find_moose_executable_recursive(base_dir)
        if exe is None:
            self.critical("Unable to locate MOOSE application executable starting in '{}'.", os.getcwd())
            return 1
        command.append(exe)

        # Locate application input file
        input_file = os.path.abspath(os.path.join(base_dir, self.getParam('input')))
        if not os.path.isfile(input_file):
            self.critical("The supplied input file '{}' does not exist in the directory '{}.", self.getParam('input'), base_dir)
            return 1
        command += ['-i', input_file]


        self.parameters().setValue('command', tuple(command))
        return runners.RunCommand.execute(self)
