import os
import sys
import subprocess

from moosetools import moosetest, mooseutils

class ExodusDiffer(moosetest.base.FileDiffer):
    """
    Compares ExodusII files.

    TODO: Add this to moosetools. This will require either adding the 'exodiff' utility to contrib
          or creating a custom python based tool. We could possible use github.com/cpgr/pyexodiff
    """
    @staticmethod
    def validParams():
        params = moosetest.base.FileDiffer.validParams()
        params.add('executable', vtype=str,
                   doc="The 'exodiff' executable to run, if not specified the version in MOOSE is used.")
        params.add('abs_zero', vtype=float, default=1e-10,
                   doc="Absolute zero cutoff used in exodiff comparisons.")
        params.add('rel_err', vtype=float, default=5.5e-6,
                   doc="Relative error value used in exodiff comparisons.")
        params.add('use_old_floor', vtype=bool, default=False,
                   doc="Enable the use of the 'old floor' option in exodiff comparisions.")
        params.add('custom_cmp', vtype=str,
                   doc="Specifies a custom comparison file using the '-f' exodiff option.")
        params.add('exodiff_opts', vtype=str,
                   doc="Additional arguments to be passed to invocations of exodiff.")
        params.add('map', vtype=bool, default=True,
                   doc=("Use geometrical mapping to match up elements. This is usually "
                        "preferred, it makes files comparable between runs with distributred "
                        "and replicated meshes."))
        params.add('partial', vtype=bool, default=False,
                   doc=("Invokes a matching algorithm similar to the -m option.  However "
                        "this option ignores unmatched nodes and elements.  This allows "
                        "comparison of files that only partially overlap."))

        return params

    def execute(self, *args):

        exe = self.getParam('executable')
        if exe is None:
            MOOSE_DIR = os.getenv('MOOSE_DIR', os.path.join(os.path.dirname(__file__), '..', '..', '..', '..'))
            exe = os.path.join(MOOSE_DIR, 'framework', 'contrib', 'exodiff', 'exodiff')
        exe = os.path.relpath(exe, os.getcwd())

        if not os.path.isfile(exe):
            msg = "The supplied 'exodiff' executable does not exist: {}"
            raise RuntimeError(msg.format(exe))

        # subprocess.run key/value arguments
        kwargs = dict()
        kwargs['capture_output'] = False
        kwargs['encoding'] = 'utf-8'
        kwargs['check'] = False # raise exceptions
        kwargs['stdout'] = subprocess.PIPE
        kwargs['stderr'] = subprocess.STDOUT

        # exodiff executable command to run
        cmd = [exe,
               '-tolerance', str(self.getParam('rel_err')),
               '-Floor', str(self.getParam('abs_zero'))]

        if self.getParam('map'): cmd.append('-map')
        if self.getParam('use_old_floor'): cmd.append('-use_old_floor')
        if self.getParam('partial'): cmd.append('-partial')

        working_dir = self.getParam('_working_dir')
        custom_cmp = self.getParam('custom_cmp')
        if custom_cmp is not None:
            cmd += ['-f', os.path.join(working_dir, custom_cmp)]

        if self.isParamValid('exodiff_opts'):
            cmd += mooseutils.separate_args(self.getParam('exodiff_opts'))

        # Perform comparison of all file pairings
        cmd += [None, None] # place holder for filenames
        for filename, gold_filename in self.pairs():
            cmd[-1] = filename
            cmd[-2] = gold_filename
            str_cmd = ' '.join(cmd)
            self.info('RUNNING EXODIFF:\n{0}\n{1}\n{0}', '-' * len(str_cmd), str_cmd)
            out = subprocess.run(cmd, **kwargs)
            self.info(out.stdout)
            if out.returncode > 0:
                self.error("{} != {}", cmd[-2], cmd[-1])
