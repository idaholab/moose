# Load the required modules
import os, sys, subprocess, socket, multiprocessing
from random import shuffle
from sets import Set

# Import the Machine and DmakeRC class
from Machine import *

## Helper function for creating machine objects, this is required to be outside
#  of the class to allow the use of running the object creation in parallel
#  @param line A string of comma sperated values containing one line of machine information from server
#  @return A Machine object generated from the given input
def createMachine(line):
  if len(line) == 0:
    return
  return Machine(*line)

## @class MachineWarehouse
#  Creates and controls the local and worker Machine objects, it mainly provides the
#  getHosts() method that returns the needed DISTCC_HOSTS environmental variable for
#  use with 'make'
# @see Machine MachineWarehouse::getHosts
class MachineWarehouse(object):


  ## Class constructor
  #
  #  Optional Arguments:
  #    allow = list<str>          - list of ip addresses/hostnames to allow in DISTCC_HOSTS
  #    restrict = list<str>       -
  #    allow_off_network = True | {False} - if true, the above restrictions are ignored
  #
  # @see DmakeRC
  def __init__(self, **kwargs):

    # Set default values for public/private members
    self.machines = []
    self.down = []

    # Build the master machine
    self.master = Machine(localhost=True)


  ## Check network
  # @param restrict A list of ip addresses to consider for local build restriction, if the
  #                 local machine starts with any of the values in this list,
  #                 erform a local build
  # @return True if the machine is on the restricted network(s)
  def onNetwork(self, restrict):

    # Assume off network
    on_network = False

    # Check the master against the list of restrited ip addresses
    for r in restrict:
      if self.master.address.startswith(r):
        on_network = True
        break

    # Return the result
    return on_network

  ## Return the hosts line and number of jobs (public)
  #  @param kwargs Optional keyword/value pairings
  #  @return DISTCC_HOSTS
  #  @return jobs
  #
  #  Optional Arguments (used by getHosts() directly):
  #    jobs = <int>             - Custom, user-defined jobs number
  #
  #  Optional Arguments (passed to buildMachines or _buildHosts):
  #    jobs = <int>             - Custom, user-defined jobs number
  #    max = True | {False}     - Run the maximum no. of threads (passed to _buildHosts)
  #    localhost = <int>        - Set the DISTCC_HOSTS localhost value (passed to _buildHosts)
  #    localslots = <int>       - Set the DISTCC_HOSTS localslots value (passed to _buildHosts)
  #    localslots_cpp = <int>   - Set the DISTCC_HOSTS localslots_cpp value (passed to _buildHosts)
  #    disable = list(<str>)    - list of machines to disable
  #    serial = True | {False}  - toggle the parallel creation of the Machine objects
  #    hammer = list(<str>)     - list of machines to set to max
  #
  #  @see _buildHosts
  def getHosts(self, host_lines, **kwargs):

    # Get the optional parameters needed by getHosts
    jobs = kwargs.get("jobs", None)

    # Buld the Machine objects
    self.buildMachines(host_lines, disable=kwargs.pop('disable', None), serial=kwargs.pop("serial", False), hammer=kwargs.pop('hammer', None))

    # Buld the hosts lines (return distcc_hosts and jobs)
    return self._buildHosts(**kwargs)


  ## Build the remote Machine objects (public)
  #  @param host_lines Raw host line data from server or .dmakerc
  #  @param disable A list of machines to disable
  #
  #  Reads the list of machines from the host_lines from the server and
  #  build the Machine objects (in parallel)
  #
  #  Optional Arguments:
  #    serial = True | {False}    - toggle the parallel creation of the Machine objects
  #    disable = list(<str>)      - a list of machines to disables
  #  @see createMachine Machine
  def buildMachines(self, host_lines, **kwargs):

    # Return if the workers are already built
    if len(self.machines) > 0:
      return

    # Handle empty host lines
    if len(host_lines) == 0:
      return

    # Get the disable list and hammer list
    disable = kwargs.pop('disable', None)
    hammer = kwargs.pop('hammer', None)

    # Create the Machine objects (in parallel)
    output = []
    if not kwargs.pop("serial", False):
      pool = multiprocessing.Pool(processes=self.master.threads)
      output = pool.map(createMachine, host_lines)
      pool.close()
      pool.join()

    # Create the Machine objects serially
    else:
      for line in host_lines:
        output.append(createMachine(line))
        output[-1].info()

    # Check the disabled list against the address and ip, the user is allowed to
    # supply partial ip/hostnames so loop through each and test that the substring
    # is not contained in either
    for machine in output:
      if hammer != None:
        for h in hammer:
          if machine.available and ((h in machine.address) or (h in machine.hostname) or (h in machine.username)):
            machine.status = 'hammer'
            machine.use_threads = machine.threads
            break

      if disable != None:
        for d in disable:
          if (d in machine.address) or (d in machine.hostname) or (d in machine.username):
            machine.status = 'disabled'
            machine.available = False
            break

    # Populate the two lists of machines
    for machine in output:

      # Available machines
      if machine.available:
        self.machines.append(machine)

      # Unavailable machines
      else:
        self.down.append(machine)


  ## Update the DISTCC_HOSTS enviornmental variable (private)
  #  @param kwargs Optional keyword/value pairings
  #
  #  Optional Arguments:
  #    max = True | {False}     - Run the maximum no. of threads (passed to _buildHosts)
  #    localhost = <int>        - Set the DISTCC_HOSTS localhost value (passed to _buildHosts)
  #    localslots = <int>       - Set the DISTCC_HOSTS localslots value (passed to _buildHosts)
  #    localslots_cpp = <int>   - Set the DISTCC_HOSTS localslots_cpp value (passed to _buildHosts)
  #    jobs = <int>             - Custom, user-defined jobs number
  def _buildHosts(self, **kwargs):

    # Extract the custom options
    has_jobs = kwargs.has_key('jobs')
    use_max = kwargs.pop('max', False)
    localhost = kwargs.pop('localhost', None)
    localslots = kwargs.pop('localslots', None)
    localslots_cpp = kwargs.pop('localslots_cpp', None)

    # Randomize the workers
    shuffle(self.machines)

    # Create the distcc_hosts and jobs output
    jobs = 0
    distcc_hosts = ''
    for machine in self.machines:
      if use_max:
        distcc_hosts += " " + machine.hostname + '/' + str(machine.threads)
        jobs += int(machine.threads)
      elif machine.use_threads != 0:
        distcc_hosts += " " + machine.hostname + '/' + str(machine.use_threads)
        jobs += machine.use_threads

    # Get the default or user-defined values for localhost/localslots_cpp/localslots
    if localhost == None:
      localhost = min(2, self.master.threads)

    if localslots == None:
      localslots = self.master.threads/4

    if localslots_cpp == None:
      localslots_cpp = self.master.threads - localhost

    # Create the DISTCC_HOSTS variable
    if jobs == 0:
      localhost = self.master.threads
      distcc_hosts = 'localhost/' + str(int(localhost))

    else:
      distcc_hosts = '--localslots=' + str(int(localslots)) + ' --localslots_cpp=' + \
                   str(int(localslots_cpp)) + ' localhost/' + str(int(localhost)) + \
                   distcc_hosts

    # Add the local machine to the jobs total
    jobs += int(localhost)

    # Get the jobs
    jobs = kwargs.pop('jobs', jobs)
    if not use_max and jobs > 70 and not has_jobs:
      jobs = 70

    # Return the values
    return distcc_hosts, jobs
