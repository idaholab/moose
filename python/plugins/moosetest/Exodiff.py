from moosetools.moosetest import runners

import subprocess

class Exodiff(runners.RunCommand):

    @staticmethod
    def validParams():
        params = runners.RunCommand.validParams()
        params.setRequired('command', False)
        return params


    def execute(self):
        kwargs = dict()
        kwargs['capture_output'] = False # use sys.stdout/sys.stderr, which is captured by TestCase
        kwargs['text'] = True # encode output to UTF-8
        kwargs['check'] = self.getParam('allow_exception')
        kwargs['timeout'] = self.getParam('timeout')



        cmd = ['/Users/slauae/projects/moose/framework/contrib/exodiff/exodiff', '-h']#self.getParam('command')
        str_cmd = ' '.join(cmd)
        self.info('RUNNING COMMAND:\n{0}\n{1}\n{0}'.format('-'*len(str_cmd) , str_cmd))
        out = subprocess.run(self.getParam('command'), **kwargs)
        return out.returncode
