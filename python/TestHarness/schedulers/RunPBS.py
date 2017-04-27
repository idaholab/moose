from MooseObject import MooseObject
from QueueManager import QueueManager
import os
## This class launches jobs using the PBS queuing system
class RunPBS(QueueManager):
    @staticmethod
    def validParams():
        params = QueueManager.validParams()
        params.addRequiredParam('scheduler',       'RunPBS',       "the name of the scheduler used")
        params.addRequiredParam('place',           'free',         "node placement")
        params.addRequiredParam('walltime',        '',             "amount of time to request for this test")
        params.addRequiredParam('template_script', 'template',     "the template script used")

        params.addParam('combine_streams',  '#PBS -j oe',           "combine stdout and stderr")
        params.addParam('pbs_queue',                  '',           "the PBS queue to use")
        params.addParam('pbs_project',                '',           "the PBS project to use")
        params.addParam('pbs_prereq',                 '',           "list of pbs jobs this job depends on")


        return params

    def __init__(self, harness, params):
        QueueManager.__init__(self, harness, params)
        self.specs = params

    # write options to qsub batch file
    def prepareLaunch(self, tester):
        return

    def getQueueCommand(self, tester):
        if self.options.processingQueue:
            return ['qstat', '-xf', self.getJobID()]
        else:
            return ['qsub', ]

    # Read the template file make some changes, and write the launch script
    def prepareQueueScript(self, preq_list):
        # Add prereq job id, now that we know them!
        if len(preq_list):
            self.specs['pbs_prereq'] = '#PBS -W depend=afterany:%s' % (':'.join(preq_list))

        f = open(self.specs['template_script'], 'r')
        content = f.read()
        f.close()

        params = self.specs

        f = open(self.queue_script, 'w')

        # Do all of the replacements for the valid parameters
        for param in params.valid_keys():
            if param in params.substitute:
                params[param] = params.substitute[param].replace(param.upper(), params[param])
            content = content.replace('<' + param.upper() + '>', str(params[param]))

        # Make sure we strip out any string substitution parameters that were not supplied
        for param in params.substitute_keys():
            if not params.isValid(param):
                content = content.replace('<' + param.upper() + '>', '')

        f.write(content)
        f.close()

    # Augment Queue specs for tester params
    def updateParams(self, tester):

        # Use the PBS template
        self.specs['template_script'] = os.path.join(tester.specs['moose_dir'], 'python/TestHarness/schedulers', 'pbs_template')

        # Convert MAX_TIME to hours:minutes for walltime use
        hours = int(int(tester.specs['max_time']) / 3600)
        minutes = int(int(tester.specs['max_time']) / 60) % 60
        self.specs['walltime'] = '{0:02d}'.format(hours) + ':' + '{0:02d}'.format(minutes) + ':00'

        if self.options.project:
            self.specs['pbs_project'] = '#PBS -P %s' % (self.options.project)

        if self.options.queue:
            self.specs['pbs_queue'] = '#PBS -q %s' % (self.options.queue)
