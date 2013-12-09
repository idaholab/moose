# Import the MachineWarehouse object
from MachineWarehouse import *


## @class MachineOutput
#  Provides screen output functionality
class MachineOutput(object):


  ## Class constructor.
  #  @param warehouse The MachineWarehouse object that output is desired from
  #  @see MachineWarehouse
  def __init__(self, warehouse):
    self.warehouse = warehouse


  ## Method for displaying the machine information (public)
  #  @param distcc_hosts The DISTCC_HOSTS envorionmental variable be used (only used with verbose=True)
  #  @param jobs The number of jobs for 'make'
  #  @kwargs Optional keyword/value pairing
  #
  # Optional Keyword/Values:
  #   verbose = True | {False} - True prints the verbose output list
  #   make_args = list(<str>)  - A list of additional arguments to be passed to make (only used with verbose=True)
  def display(self, distcc_hosts, jobs, **kwargs):
    if kwargs.pop('verbose', False):
      self._verbose(distcc_hosts, jobs, **kwargs)
    else:
      self._standard(jobs)


  ## Display the non-verbose output (private)
  #  @param jobs The number of processors being run
  def _standard(self, jobs):
    print 'Number of processors: ' + str(jobs)


  ## Display the verbose output (private)
  #  @param distcc_hosts The DISTCC_HOSTS envorionmental variable be used (only used with verbose=True)
  #  @param jobs The number of jobs for 'make'
  #  @kwargs Optional keyword/value pairing
  #
  # Optional Keyword/Values:
  #   make_args = list(<str>)  - A list of additional make arguements
  def _verbose(self, distcc_hosts, jobs, **kwargs):

    # Extract the optional, additional make arguments
    make_args = kwargs.pop('make_args', [])

    # Setup the output list variable
    output = []

    # Print the headings
    output.append('\nMACHINE INFORMATION:')

    # Get max widths (dict with attribute as keyword)
    col_width = self._getMaxColumnWidth()
    headings  = ['Status', 'Username', 'Description', 'Jobs', 'Hostname', 'IP Address', 'Last Used']
    attributes = ['status', 'username', 'description', 'use_threads', 'hostname', 'address', 'timestamp']

    # Update the column widths to account for the width of the headings
    for i in range(len(attributes)):
      if len(headings[i]) > col_width[attributes[i]]:
        col_width[attributes[i]] = len(headings[i])

    # Build the formatted string for the title output
    output.append('| ')
    for i in range(len(attributes)):
      if attributes[i] in col_width:
        w = str(col_width[attributes[i]]+2)
        frmt = ' %-' + w + 's|'
        output[-1] += frmt % headings[i]

    # Insert the heading horizontal lines
    s = '+' + '-'*(len(output[1])-2) + '+'
    output.insert(1,s)   # top line
    output.append(s)     # under headings

    # Print the master machine information
    output.append(self._formatter(self.warehouse.master, col_width, attributes))

    # Print the slave machine information, if the information exists
    use = 0
    total = 0
    if len(self.warehouse.workers) > 0:
      for machine in self.warehouse.workers:
         output.append(self._formatter(machine, col_width, attributes))
         use += machine.use_threads
         total += machine.cores
      output.append(s)

    # Print the down machine information, if the informatin exists
    if len(self.warehouse.down) > 0:
      for machine in self.warehouse.down:
        output.append(self._formatter(machine, col_width, attributes))
      output.append(s)

    # Make sure the table is closed
    if output[-1] != s:
      output.append(s)

    # Print header for building information
    output.append('\nBUILD INFROMATION:')

    # Format string for information display
    frmt = '%25s: '

    # Print processors
    output.append(frmt % 'Processors Used' + str(jobs))

    # Total available
    output.append(frmt % 'Available Processors' + str(use+self.warehouse.master.use_threads))
    output.append(frmt % 'Total Processors' + str(total+self.warehouse.master.threads))

    # Add make command
    make = frmt % 'Make Command' + 'make -j ' + str(jobs)
    for a in make_args:
      make += ' ' + a
    output.append(make)

    # Display DISTCC_HOSTS
    output.append(frmt % 'DISTCC_HOSTS' + distcc_hosts + '\n')

    # Print the output
    for line in output:
      print line


  ## Creates the formatted strings (private)
  #  @param machine The machine object for which the data is desired
  #  @param widths The dict() containing the column widths stored by attribute name
  #  @param attributes A list of attributes desired
  def _formatter(self, machine, widths, attributes):
    output = '| '
    for key in attributes:
      value = str(getattr(machine, key))
      frmt = ' %-' + str(widths[key]+2) + 's|'
      output += frmt % value
    return output


  ## Compute the maximum column width (private)
  #  Loop throughs all machine objects and stores the largest column width
  #  @return A dictionary of column widths for each Machine attribute
  def _getMaxColumnWidth(self):

    # Get the widths for the local machine
    widths = self.warehouse.master.getLengths()

    # Loop through all of the keys in the online and offline machines
    # and store the max value
    for key in widths:
      for machine in self.warehouse.workers:
        m_widths = machine.getLengths()
        widths[key] = max(widths[key], m_widths[key])
      for machine in self.warehouse.down:
        m_widths = machine.getLengths()
        widths[key] = max(widths[key], m_widths[key])

    # Output the dict
    return widths
