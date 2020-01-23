#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from FactorySystem import InputParameters
import os, sys, shutil

# Get the real path of cluster_launcher
if(os.path.islink(sys.argv[0])):
    pathname = os.path.dirname(os.path.realpath(sys.argv[0]))
else:
    pathname = os.path.dirname(sys.argv[0])
    pathname = os.path.abspath(pathname)

# Add the utilities/python_getpot directory
MOOSE_DIR = os.path.abspath(os.path.join(pathname, '../'))
FRAMEWORK_DIR = os.path.abspath(os.path.join(pathname, '../../', 'framework'))
#### See if MOOSE_DIR is already in the environment instead
if os.environ.get("MOOSE_DIR"):
    MOOSE_DIR = os.environ['MOOSE_DIR']
    FRAMEWORK_DIR = os.path.join(MOOSE_DIR, 'framework')
if os.environ.get("FRAMEWORK_DIR"):
    FRAMEWORK_DIR = os.environ['FRAMEWORK_DIR']

class Job(object):
    def validParams():
        params = InputParameters()
        params.addRequiredParam('type', "The type of test of Tester to create for this test.")
        params.addParam('template_script', MOOSE_DIR + '/python/ClusterLauncher/pbs_submit.sh', "The template job script to use.")
        params.addParam('job_name', 'The name of the job')
        params.addParam('test_name', 'None', 'The name of the test')
        return params
    validParams = staticmethod(validParams)

    def __init__(self, name, params):
        self.specs = params

    # Called from the current directory to copy files (usually from the parent)
    def copyFiles(self, job_file):
        for file in os.listdir('../'):
            if os.path.isfile('../' + file) and file != job_file:
                shutil.copy('../' + file, '.')

    # Called to prepare a job script if necessary
    def prepareJobScript(self):
        return

    # Called to launch the job
    def launch(self):
        return
