#!/usr/bin/env python
import os, sys, shutil, subprocess

MOOSE_DIR = '../'
#### See if MOOSE_DIR is already in the environment instead
if os.environ.has_key("MOOSE_DIR"):
  MOOSE_DIR = os.environ['MOOSE_DIR']
sys.path.append(MOOSE_DIR + '/tests')
import ParseGetPot

JOB_LIST = 'job_list'

def parseJobsFile(template_dir):
  jobs = []
  # We expect the job list to be named "job_list"
  filename = template_dir + JOB_LIST

  try:
    data = ParseGetPot.readInputFile(filename)
  except:        # ParseGetPot class
    print "Parse Error: " + filename
    return jobs

  # We expect our root node to be called "Jobs"
  if 'Jobs' in data.children:
    jobs_node = data.children['Jobs']

    for jobname, job_node in jobs_node.children.iteritems():
      job = {}
      job['job_name'] = jobname
      for key, value in job_node.params.iteritems():
        job[key] = value

      jobs.append(job)
  return jobs

def createAndLaunchJob(template_dir, specs):
  if os.path.exists(template_dir + specs['job_name']):
    print "Error: Job directory", template_dir + specs['job_name'], "already exists"
    sys.exit(1)

  # Make directory
  os.mkdir(template_dir + specs['job_name'])
  saved_cwd = os.getcwd()
  os.chdir(template_dir + specs['job_name'])

  # Copy files
  for file in os.listdir('../'):
    if os.path.isfile('../' + file) and file != JOB_LIST:
      shutil.copy('../' + file, '.')

  # Populate variable replacements
  job_name = specs['job_name']
  nodes = specs['nodes']
  mpi_procs = specs['mpi_procs']
  threads = specs['threads']

  cpus_per_node = str(int(mpi_procs) * int(threads))
  total_cpus = str(int(cpus_per_node) * int(nodes))

  # Look for the shell script and modify appropriately
  for file in os.listdir('.'):
    if os.path.isfile(file) and file[-3:] == '.sh':
      f = open(file, 'r+')

      # Replace variables in the shell file
      content = f.read()
      content = content.replace('<NODES>', nodes)
      content = content.replace('<MPI_PROCS>', mpi_procs)
      content = content.replace('<THREADS>', threads)
      content = content.replace('<CPUS_PER_NODE>', cpus_per_node)
      content = content.replace('<TOTAL_CPUS>', total_cpus)
      content = content.replace('<JOB_NAME>', job_name)

      # Write the contents back to the file
      f.seek(0)
      f.write(content)
      f.truncate()
      f.close()

      # Finally launch the job
      subprocess.Popen('qsub ' + file, shell=True)

  os.chdir(saved_cwd)


def main():
  if len(sys.argv) != 2:
    print "Usage:", sys.argv[0], " <template directory>"
    sys.exit(1)

  template_dir = os.path.abspath(sys.argv[1]) + '/'
  jobs = parseJobsFile(template_dir)

  for job in jobs:
    createAndLaunchJob(template_dir, job)

if __name__ == '__main__':
  main()
