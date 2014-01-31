#!/usr/bin/env python

# Load the required modules
import os, sys, urllib2, socket, subprocess, multiprocessing, platform, datetime

## @class Machine
#  Each computer in the distcc pool has attributes stored in an instance of the Machine class,
#  which contians all the necessary information for adding the machine to the allow list
#  for distccd and the DISTCC_HOSTS environmental variable for compiling with distcc
#
#  In general, all interaction with the various Machine objects is handled via the MachineWarehouse
#  class, which handles the creation and utilization of the objects
#  @see MachineWarehouse
class Machine(object):


  ## Machine class constructor
  #  @param args The machine parts as read from the server, these are passed
  #             to _initRemote
  #  @param kwargs Optional set of keyword/value pairings
  #
  #  Optional Keyword Input (kwargs):
  #   localhost = True | {False}
  #     When set to True the local Machine object is created, the args input is not utilized.
  def __init__(self, *args, **kwargs):

    # Initialize the available fields (public)
    self.address = None
    self.cores = None
    self.threads = None
    self.use_threads = None
    self.network = None
    self.description = None
    self.timestamp = None
    self.username = None
    self.hostname = None
    self.available = None
    self.status = 'offline'
    self.dedicated = False

    # The machine is from the localhost
    localhost = kwargs.pop('localhost', False)
    if localhost:
      self._initLocalhost()
      self.available = True

    # The machine is a remote
    else:
      self._initRemote(*args)
      self.available = self._isAvailable()

    # Attempt to get the host name from ip, if it is unknown
    try:
      h = socket.gethostbyaddr(self.address)
      self.hostname = h[0]
    except:
      self.hostname = self.address
      #self.available = False

    # Set the status
    if self.available:
      if (int(self.threads) / 2) == self.use_threads:
        self.status = 'dedicated'
        self.dedicated = True
      else:
        self.status = 'online'

    # Compute lengths of attributes, this is used by the MachineWarehouse
    # for creating the output table
    self._length = dict()
    self._computeLength()


  ## Return a dictionary containing the number of characters in each attricute (public)
  #  @return Dictionary with attribute name as keyword and the number of characters of the attriubte as the value
  #
  #  For example, the length of the hostname attribute would be retrieved as follows:
  #   L = machine.getLenghts()
  #   h = L['hostname']
  def getLengths(self):
    return self._length


  ## Prints the attributes for this machine (public)
  #  Displays a simple list of the attributes and their respective values,
  #  mainly for debugging purposes.
  def info(self):
    print '\n' + self.description + ':'
    for member in self._getMembers():
      if member != 'description':
        print '  ' +  member + ' = ' + str(getattr(self, member))


  ## Method for extracting names of "public" member variables (private)
  #  @return List of "publc" attribute names, i.e. the attributes without an underscore prefix
  def _getMembers(self):

    # Initialize the output list
    output = []

    # Loop through each member and store those without underscore
    for member in self.__dict__.keys():
      if not member.startswith('_'):
        output.append(member)
    return output


  ## Method for initializing the local Machine object (private)
  #  Automatically fills in the various attributes of the Machine using Python built-in methods
  def _initLocalhost(self):

    # Get the IP address of this machine, if on linux you need to get the IP address in special way to
    # avoid getting 127.0.0.1 for the local machine
    if platform.system().lower() == 'linux':
      s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
      s.connect(('inl.gov', 0))
      self.address = s.getsockname()[0]
    else:
      self.address = socket.gethostbyname(socket.gethostname())

    # The number of cores is not needed
    self.cores = None

    # Set the number of threads and the number of threads to use to the total available threads
    self.threads = multiprocessing.cpu_count()
    self.use_threads = self.threads

    # Set the network
    if self.address.find('204.') != -1:
      self.network = 'fn'
    else:
      self.network = 'inl'
      os.environ['no_proxy'] = '.inl.gov'

    # Set the description, if the remote or local file is read this will be updated with the user supplied
    # description. This is not done here for speed purposed if a local only build is performed
    self.description = 'localhost'
    self.username = os.getenv('USER')

    # Set the time to the current time
    self.timestamp = datetime.datetime.now().strftime('%m/%d/%Y %H:%M')


  ## Method for initilizing remote machines objects (private)
  #  Create the remote machine object based on the attributes passed in
  #  @param address The IP address of the remote machine
  #  @param cores The number of real CPU cores for the remote machine
  #  @param threads The maximum number of threads for the remote machine
  #  @param use_threads The number of threads allowed to be used
  #  @param network The type of network of the machine ('inl' or 'fn')
  #  @param description The user supplied description of the remote machine
  #  @param timestamp The time that the machine was last used (optional)
  #  @param username The username of the remote machine (optional)
  def _initRemote(self, *args):

    # Set the various attributes of the machine
    self.address = args[0]
    self.cores = int(args[1])
    self.threads = int(args[2])
    self.use_threads = int(args[3])
    self.network = args[4]
    self.description = args[5]

    # Add the timestamp, if it exists
    if len(args) > 6:
      self.timestamp = args[6]

    # Add the username, if it exists
    if len(args) > 7:
      self.username = args[7]


  ## Test if the machine is available (private)
  #  Utilizes lsdistcc command to test if the machine is available for
  #  use by the distcc compilier.
  def _isAvailable(self):

    # Run lsdistcc with the given hostname
    sub = subprocess.Popen(['lsdistcc', '-x', self.address], bufsize=0,\
                           stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    status = sub.communicate()[0]

    # If empty or contians ',down' then the host is not available
    if 'down' in status:
      return False
    else:
      return True


  ## Compute the length of the various attributes (private)
  #  Creates the dictionary of attributes and the associated
  #  lengths of their values.
  #  @see getLenghts
  def _computeLength(self):
    for member in self._getMembers():
      self._length[member] = len(str(getattr(self, member)))
