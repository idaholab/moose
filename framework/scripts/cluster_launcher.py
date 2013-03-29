#!/usr/bin/env python
import os, sys, re, shutil

if os.environ.has_key("MOOSE_DIR"):
  MOOSE_DIR = os.environ['MOOSE_DIR']
else:
  MOOSE_DIR = os.path.abspath(os.path.dirname(sys.argv[0])) + '/..'
sys.path.append(MOOSE_DIR + '/scripts/common')
sys.path.append(MOOSE_DIR + '/scripts/cluster_launcher')

import ParseGetPot
from InputParameters import InputParameters
from Factory import Factory
from PBSJob import PBSJob

class ClusterLauncher:
  def __init__(self, template_dir):
    self.factory = Factory()
    self.job_list = 'job_list'
    self.template_dir = template_dir

  def parseJobsFile(self):
    jobs = []
    # We expect the job list to be named "job_list"
    filename = self.template_dir + self.job_list

    try:
      data = ParseGetPot.readInputFile(filename)
    except:        # ParseGetPot class
      print "Parse Error: " + filename
      return jobs

    # We expect our root node to be called "Jobs"
    if 'Jobs' in data.children:
      jobs_node = data.children['Jobs']

      for jobname, job_node in jobs_node.children.iteritems():
        # First retrieve the type so we can get the valid params
        if 'type' not in job_node.params:
          print "Type missing in " + filename
          sys.exit(1)

        params = self.factory.getValidParams(job_node.params['type'])

        params['job_name'] = jobname

        # Now update all the base level keys
        params_parsed = set()
        params_ignored = set()
        for key, value in job_node.params.iteritems():
          params_parsed.add(key)
          if key in params:
            if params.type(key) == list:
              params[key] = value.split(' ')
            else:
              if re.match('".*"', value):  # Strip quotes
                params[key] = value[1:-1]
              else:
                params[key] = value
          else:
            params_ignored.add(key)

        # Make sure that all required parameters are supplied
        required_params_missing = params.required_keys() - params_parsed
        if len(required_params_missing):
          print 'Required Missing Parameter(s): ', required_params_missing
          sys.exit(1)
        if len(params_ignored):
          print 'Ignored Parameter(s): ', params_ignored

        jobs.append(params)
    return jobs

  def createAndLaunchJob(self, specs):
    if os.path.exists(self.template_dir + specs['job_name']):
      print "Error: Job directory", self.template_dir + specs['job_name'], "already exists"
      sys.exit(1)

    # Make directory
    os.mkdir(self.template_dir + specs['job_name'])
    saved_cwd = os.getcwd()
    os.chdir(self.template_dir + specs['job_name'])

    # Copy files
    for file in os.listdir('../'):
      if os.path.isfile('../' + file) and file != self.job_list:
        shutil.copy('../' + file, '.')

    # Files have been copied so turn the remaining work over to the Job instance
    job_instance = self.factory.create(specs['type'], specs)

    # Prepare the Job Script
    job_instance.prepareJobScript()

    # Launch it!
    job_instance.launch()

    os.chdir(saved_cwd)

  def registerJobType(self, type, name):
    self.factory.register(type, name)

  def run(self):
    jobs = self.parseJobsFile()

    for job in jobs:
      self.createAndLaunchJob(job)


########################################################
def main():
  if len(sys.argv) != 2:
    print "Usage:", sys.argv[0], " <template directory>"
    sys.exit(1)

  template_dir = os.path.abspath(sys.argv[1]) + '/'

  cluster_launcher = ClusterLauncher(template_dir)
  cluster_launcher.registerJobType(PBSJob, 'PBSJob')
  cluster_launcher.run()


if __name__ == '__main__':
  main()
