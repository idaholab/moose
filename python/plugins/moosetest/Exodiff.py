import os
import sys
import subprocess

import ast

from moosetools.moosetest.base import Differ, make_differ, Runner
from moosetools.moosetest.differs import ConsoleDiff
from .MooseAppRunner import MooseAppRunner
from . import helpers


class FileDifferBase(Differ):

    def validParams():
        params = Differ.validParams()
        params.add('gold_filenames', vtype=str, array=True,
                   doc="")
        params.add('gold_dir', vtype=str, default='gold',
                   doc="")
        return params

    def __init__(self, *args, **kwargs):
        Differ.__init__(self, *args, **kwargs)
        self.__filename_pairs = list()


    def preExecute(self):
        Differ.preExecute(self)

        filenames = Runner.filenames(self)

        if self.isParamValid('gold_filenames'):
            gold_filenames = Runner.filenames(self, 'gold_filenames')

        else:
            gold_dir = self.getParam('gold_dir')
            gold_filenames = list()
            for filename in filenames:
                d, f = os.path.split(filename)
                gold_filenames.append(os.path.join(d, gold_dir, f))

        if len(filenames) != len(gold_filenames):
            msg = "The number of supplied file(s) for comparison are not the same length:\nFile(s):\n{}\nGold File(s):\n{}"
            self.error(msg, '\n'.join(filenames), '\n'.join(gold_filenames))

        self.__filename_pairs = [(f, g) for f, g in zip(filenames, gold_filenames)]


    def pairs(self):
        for f, g in self.__filename_pairs:
            yield f, g


# TODO: Create FileDifferBase in moosetools
#       Add this to moosetools, requires adding exodiff utility to contrib
class ExodusDiff(FileDifferBase):
    @staticmethod
    def validParams():
        params = FileDifferBase.validParams()
        #params.add('executable')
        return params




    def execute(self, *args):

        exe = '/Users/slauae/projects/moose/framework/contrib/exodiff/exodiff'


        kwargs = dict()
        kwargs['capture_output'] = True
        kwargs['encoding'] = 'utf-8'
        kwargs['check'] = True # raise exceptions

        rcode = 0
        for filename, gold_filename in self.pairs():
            cmd = [exe, filename, gold_filename]
            str_cmd = ' '.join(cmd)
            print('RUNNING EXODIFF:\n{0}\n{1}\n{0}'.format('-' * len(str_cmd), str_cmd))
            out = subprocess.run(cmd, **kwargs)
            sys.stdout.write(out.stdout)
            sys.stderr.write(out.stderr)
            rcode += out.returncode

        return int(rcode > 0)


class Exodiff(MooseAppRunner):

    @staticmethod
    def validParams():
        params = MooseAppRunner.validParams()
        params.add('exodiff', array=True, vtype=str,
                   doc="ExodusII file(s) to compare with counterpart in 'gold' directory.")
        return params

    def __init__(self, *args, **kwargs):
        MooseAppRunner.__init__(self, *args, **kwargs)

        controllers = self.getParam('_controllers')
        differs = list()

        #print(self.parameters())

        kwargs = dict()
        kwargs['filenames'] = self.getParam('exodiff')
        kwargs['base_dir'] = self.getParam('base_dir')

        exo_differ = make_differ(ExodusDiff, controllers, name='exodiff', **kwargs)
        self.parameters().setValue('differs', (exo_differ,))

   # def execute(self):
   #     pass

class PetscJacobianTester(MooseAppRunner):
    @staticmethod
    def validParams():
        params = MooseAppRunner.validParams()

        params.add('ratio_tol', default=1e-8, doc="Relative tolerance to compare the ration against.")
        params.add('difference_tol', default=1e-8, doc="Relative tolerance to compare the difference against.")
        params.add('state', default='user', doc="The state for which we want to compare against the "
                                         "finite-differenced Jacobian ('user', 'const_positive', or "
                                         "'const_negative'.")
        params.add('run_sim', default=False, doc="Whether to actually run the simulation, testing the Jacobian "
                                          "at every non-linear iteration of every time step. This is only "
                                          "relevant for petsc versions >= 3.9.")
        params.add('turn_off_exodus_output', default=True, doc="Whether to set exodus=false in Outputs")
        params.add('only_final_jacobian', default=False, doc="Check only final Jacobian comparison.")

        return params

    def execute(self):
        #self.error('Not Done')
        return 0


class RunApp(MooseAppRunner):
    @staticmethod
    def validParams():
        params = MooseAppRunner.validParams()
        params.add('absent_out', vtype=str,
                   doc="Ensure that the supplied text is not found the output text.")
        return params

    def __init__(self, *args, **kwargs):
        MooseAppRunner.__init__(self, *args, **kwargs)

        controllers = self.getParam('_controllers')
        differs = list()

        kwargs = dict()
        kwargs['base_dir'] = self.getParam('base_dir')
        kwargs['re_not_match'] = self.getParam('absent_out')

        c_differ = make_differ(ConsoleDiff, controllers, name='consolediff', **kwargs)
        print(self.getParam('absent_out'), c_differ.getParam('re_not_match'))


        self.parameters().setValue('differs', (c_differ,))
