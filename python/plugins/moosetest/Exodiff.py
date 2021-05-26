import sys
import subprocess

from moosetools.moosetest.base import Differ, make_differ
from .MooseAppRunner import MooseAppRunner
from . import helpers

# TODO: Create FileDifferBase in moosetools
#       Add this to moosetools, requires adding exodiff utility to contrib
class ExodusDiff(Differ):
    @staticmethod
    def validParams():
        params = Differ.validParams()
        params.add('executable')

        # TODO: FileDiffer
        params.add('files')
        params.add('gold_files') # if not provided use base_dir/gold_dir/files
        params.add('gold_dir')
        return params

    def execute(self, *args):


        base_dir = helpers.get_file_base(self)
        filenames = list()
        for f in self.getParam('files'):
            filename = os.path.abspath(os.path.join(base_dir, f))
            if not os.path.isfile(filename):
                msg = "The supplied file '{}' does not exist in the directory '{}'."
                self.error(msg, f, base_dir)
            else:
                filenames.append(filename)




        cmd = ['/Users/slauae/projects/moose/framework/contrib/exodiff/exodiff', '-h']

        kwargs = dict()
        kwargs['capture_output'] = True
        kwargs['encoding'] = 'utf-8'
        kwargs['check'] = True # raise exceptions
        #kwargs['timeout'] = 20 # ???

        #md = self.getParam('command')

        str_cmd = ' '.join(cmd)
        self.info('RUNNING EXODIFF:\n{0}\n{1}\n{0}'.format('-' * len(str_cmd), str_cmd))
        out = subprocess.run(cmd, **kwargs)
        sys.stdout.write(out.stdout)
        sys.stderr.write(out.stderr)

        self.error("foo")

        #return out.returncode








class Exodiff(MooseAppRunner):

    @staticmethod
    def validParams():
        params = MooseAppRunner.validParams()
        params.add('exodiff')
        return params

    def __init__(self, *args, **kwargs):
        MooseAppRunner.__init__(self, *args, **kwargs)

        controllers = self.getParam('_controllers')
        differs = list()
        differs.append(make_differ(ExodusDiff, controllers, name='exodiff'))

        self.parameters().setValue('differs', tuple(differs))

    def execute(self):
        pass
