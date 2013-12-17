# Load requied packages
import sys, os, pickle, uuid, platform, urllib2
from sets import Set

## @class DmakeRC
#  Manages the .dmakrc file, which stores various information mainly for the purpose
#  of using a cached verision of the DISTCC_HOSTS enviornmental variable for the
#  reason of speeding up the build process.
#
#  Gernerally, the MachineWarehouse automatically manages all aspects of the .dmakerc file
#  via the interface provided by this object.
#
#  @see MachineWarehouse
class DmakeRC(object):

  ## Class constructor
  #  @param master The master machine object
  #  @param kwargs Optional keyword/value parings
  #
  #  Optional Keyword/Value input:
  #    buck = True | {False}      - True to set 'buck' as the remote server
  #    dedicated = True | {False} - True sets the local machine as a dedicated build box on the server
  #    description = <str>        - Set the description of this computer
  #    clean = True | {False}     - remove existing .dmakerc
  def __init__(self, master, **kwargs):

    # Extract the options from the user
    self._test = kwargs.pop('buck', False)
    self._dedicated = kwargs.pop('dedicated', False)

    # Store the local Machine object
    self._master = master

    # Store the complete filename for the .dmakerc file
    self._filename =  os.path.join(os.getenv('HOME'), '.dmakerc')

    # Set the local flag (this is set to true if the remote connection fails, the
    # MachineWarehouse will then use a local build)
    self._fail = False

    # Remove .dmakerc if clean option is given
    if kwargs.pop('clean', False):
      if os.path.exists(self._filename):
        os.remove(self._filename)

    # Initialize the update variable, this value is True when when
    # the DISTCC_HOSTS stored is out-of-date
    self._update_distcc_hosts = False

    # Read the .dmakerc, stored as a dictionary which is accessible via
    # the get/set methods of this object
    self._dmakerc = self._readLocalFile()

    # Set/update the user supplied description of the local machine
    self._checkDescription(kwargs.pop('description', None))

    # Read the remote machine file from the server, if nothing has changed
    # this list will be identical to the HOST_LINES item in the self._dmakerc
    # dictionary
    self._remote  = self._readRemoteFile()

    # Return if the remote connection fails
    if self._remote == None:
      self._fail = True
      return

    # Set the stored HOST_LINES to be the same as the server
    self.set(HOST_LINES=self._remote)


  ## Accessor for stored variables (public)
  #  @param key The name of the dictionary item to retrun (e.g., 'DISTCC_HOSTS')
  #  @return The value from the dictionary for the given key
  def get(self, key):
    return self._dmakerc[key]


  ## Method for storing data (public)
  #  @param kwargs Key/value pairings that include the variables to set as well
  #                as optional triggers
  #
  #  Available keywords for storing data:
  #    DISTCC_HOSTS = <str>     - String containing the complete DISTCC_HOSTS enviornmental variable
  #    JOBS = <int>             - No. of processes to utilize for 'make'
  #    DESCRIPTION = <str>      - String containing description of this machine
  #    HOST_LINES = list(<str>) - A list of raw strings, with each entry representing a remote machine, as
  #                               read from the server
  #
  #  Optional keywords available:
  #    write = {True} | False   - Toggle the writting of the .dmakerc file if something is changes
  #
  #  Example:
  #    set(DISTCC_HOSTS=hosts, JOBS=4, write=False)
  def set(self, **kwargs):

    # Flag for writting file
    write = kwargs.pop('write', True)

    # Loop through the keyword/value pairs and update the store
    # values if they are different
    for key, value in kwargs.iteritems():
     if self._dmakerc[key] != value:
       self._dmakerc[key] = value

       # If the HOST_LINES is changed, the DISTCC_HOSTS needs updated,
       # the actual updating is handled by the MachineWarehouse
       if key == 'HOST_LINES':
         self._update_distcc_hosts = True

    # Store the change to the file (this is allways done unless specified)
    if write:
      fid = open(self._filename, 'w')
      pickle.dump(self._dmakerc, fid)
      fid.close()


  ## Was the .dmakerc file updated (public)
  #  @return True if the DISTCC_HOSTS variable needs to be updated
  def distccHostsNeedsUpdate(self):
    if self.get('DISTCC_HOSTS') == None:
      return True
    else:
      return self._update_distcc_hosts


  ## Return the status of the remote access
  # @return True if the remote connection failed
  def fail(self):
    return self._fail


  ## Reads the local .dmakerc file (private)
  # @return A map of the data loaded from the .dmakerc
  def _readLocalFile(self):

    # Read the file, if it exists
    if os.path.exists(self._filename):
      fid = open(self._filename, 'r')
      data = pickle.load(fid)
      fid.close()
    else:
      data = dict()

    # Set the default values for the keys in the .dmakerc
    data.setdefault('HOST_LINES', None)
    data.setdefault('DESCRIPTION', None)
    data.setdefault('DISTCC_HOSTS', None)
    data.setdefault('JOBS', None)

    # Clean up the hostlines
    if data['HOST_LINES'] != None:
      data['HOST_LINES'] = self._cleanHostLines(data['HOST_LINES'])

    # Return the data
    return data


  ## Reads the remote machine list (private)
  # @return A map of the data loaded from the .dmakerc
  def _readRemoteFile(self):

    # Define the servers
    buck =  dict(url='https://buck.inl.gov/distcc_gen', name='buck')
    hpcsc = dict(url='https://hpcsc.inl.gov/distcc_gen', name='hpcsc')

    # Get the remote location to utilize
    if self._test:
      remote = buck
      backup = hpcsc
    else:
      remote = hpcsc
      backup = buck

    # Set the 'use_threads' attribute for this machine
    if self._dedicated:
      self._master.use_threads = (int(self._master.threads) / 2)
    else:
      self._master.use_threads = (int(self._master.threads) / 4)

    # Read the data from the server
    data = self._urlOpen(remote)

    # If data doesn't exist try the backup
    if data == None:
      data = self._urlOpen(backup)

    # If it still dosen't exist, run locallay
    if data == None:
      return None

    # Return the host lines from the remote server
    return self._cleanHostLines(data)


  ## Create/override the user supplied description (private)
  #  @param description A description of this machine
  def _checkDescription(self, description):

    # Set the description via the input argument
    if description != None:
      self.set(DESCRIPTION = description[0])

    # If the description is empty, prompt the user
    if self.get('DESCRIPTION') == None:
      print 'Supply a description of your machine:'
      while self.get('DESCRIPTION') == None:
        self.set(DESCRIPTION=sys.stdin.readline().split('\n')[0])

    # Update the description of this machine
    self._master.description = self.get('DESCRIPTION')
    self._master._computeLength()


  ## Cleans up the raw HOST_LINES input (private)
  #  @param input The list of raw HOST_LINES from the server or .dmakerc file
  #  @return A sorted list that does not contain duplicate IP addresses
  def _cleanHostLines(self, input):

    # Create an empty set for storing unique IP addresses
    ip = Set()

    # Remove any empty rows from the raw file
    input = filter(None, input)

    # Initialize output list
    output = []

    # Loop throught the input
    for line in input:

      # Extract the IP address
      address = line.split(',')[0]

      # If IP address has not been encountered and it is not this machine and it is not 127.0.0.1
      # add the raw line to the output
      if (address not in ip) and (address != self._master.address) and (address != '127.0.0.1'):
        output.append(line)
        ip.add(address)

    # Sort the output and return it
    output.sort()
    return output


  ## Access the server
  # @param input Dictionary with url and name parameters for the remote server
  # @return The data from the server if sucessfull, or None if it fails
  def _urlOpen(self, input):

    # Set the url to send to the remote server
    filename = input['url'] + '?uuid=' + str(uuid.uuid3(uuid.NAMESPACE_DNS, self._master.hostname)) + \
                 '&arch=' + platform.system().lower() + \
                 '&ip=' + self._master.address + \
                 '&cpus=' + str(self._master.threads) + \
                 '&threads=' + str(self._master.threads) + \
                 '&use=' + str(self._master.use_threads) + \
                 '&network=' + self._master.network + \
                 '&description=' + urllib2.quote(self._master.description) + \
                 '&username=' + os.getenv('USER')

    try:
      fid = urllib2.urlopen(filename, None, 1)
      data = fid.read().split('\n')
      fid.close()
      return data

    except:
      print 'Warning: Failed to connect to ' + input['name']
      return None
