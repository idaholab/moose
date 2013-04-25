#!/usr/bin/env python
import os, sys, re, shutil
from optparse import OptionParser, OptionGroup, Values

if os.environ.has_key("MOOSE_DIR"):
  MOOSE_DIR = os.environ['MOOSE_DIR']
else:
  MOOSE_DIR = os.path.abspath(os.path.dirname(sys.argv[0])) + '/..'
sys.path.append(MOOSE_DIR + '/scripts/common')
sys.path.append(MOOSE_DIR + '/scripts/ClusterLauncher')

import ParseGetPot
from InputParameters import InputParameters
from Factory import Factory
from PBSJob import PBSJob

class ClusterLauncher:
  def __init__(self):
    self.factory = Factory()
    self.job_list = 'job_list'

  def parseJobsFile(self, template_dir):
    jobs = []
    # We expect the job list to be named "job_list"
    filename = template_dir + self.job_list

    try:
      data = ParseGetPot.readInputFile(filename)
    except:        # ParseGetPot class
      print "Parse Error: " + filename
      return jobs

    # We expect our root node to be called "Jobs"
    if 'Jobs' in data.children:
      jobs_node = data.children['Jobs']

      # Get the active line
      active_jobs = None
      if 'active' in jobs_node.params:
        active_jobs = jobs_node.params['active'].split(' ')

      for jobname, job_node in jobs_node.children.iteritems():
        # Make sure this job is active
        if active_jobs != None and not jobname in active_jobs:
          continue

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

  def createAndLaunchJob(self, template_dir, specs):
    if not os.path.exists(template_dir + specs['job_name']):
      # Make directory
      os.mkdir(template_dir + specs['job_name'])
    elif not specs['allow_multiple']:
      print "Error: Job directory", template_dir + specs['job_name'], "already exists"
      return

    saved_cwd = os.getcwd()
    os.chdir(template_dir + specs['job_name'])

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

  ### Parameter Dump ###
  def printDump(self):
    self.factory.printDump("Jobs")
    sys.exit(0)

  def run(self, template_dir):
    jobs = self.parseJobsFile(template_dir)

    for job in jobs:
      self.createAndLaunchJob(template_dir, job)

########################################################
def main():
  parser = OptionParser(usage='Usage: %prog [options] <template directory>')
  parser.add_option("--dump", action="store_true", dest="dump", default=False, help="Dump the parameters for the testers in GetPot Format")
  (options, dir) = parser.parse_args()

  cluster_launcher = ClusterLauncher()
  cluster_launcher.registerJobType(PBSJob, 'PBSJob')

  if options.dump:
    cluster_launcher.printDump()

  if not dir:
    parser.print_help()
    sys.exit(1)

  template_dir = os.path.abspath(dir[0]) + '/'
  cluster_launcher.run(template_dir)

if __name__ == '__main__':
  main()
