#!/usr/bin/python
import os, sys, csv

# Peacock modules
sys.path.append(os.path.abspath('..'))
from base import *

##
# A class for performing read/write operations on *.csv files
class CSVIO(object, PeacockErrorInterface, PeacockTestInterface):

  ##
  # Class constructor
  # @param filename The name of the csv file top open
  # @param kwargs Optional keyword/value pairs
  #
  # Optional Arguments:
  #   @see PeacockErrorInterface, PeacockTestInterface
  def __init__(self, filename, **kwargs):
    PeacockErrorInterface.__init__(self, **kwargs)
    PeacockTestInterface.__init__(self, **kwargs)

    # Initialize member variables
    self._headers = []
    self._data = dict()

    # Produce an error if the file does not exist
    if not os.path.isfile(filename):
      self.peacockError('The file \'' + filename + '\' does not exist.')
      return

    # Read the file
    with open(filename) as csvfile:
      reader = csv.reader(csvfile)

      # Extract the data into a dictionary
      self._data = dict()
      on_header = True

      # Loop through the rows
      for row in reader:

        # Store the header and initialize the data dictionary
        if on_header:
          headers = row
          for h in headers:
            key = h.strip()
            self._headers.append(key)
            self._data[key] = []
          on_header = False


        # Extract the data
        else:
          for idx in xrange(len(row)):
            self._data[self._headers[idx]].append(float(row[idx]))

  ##
  # Operator[] to allow access to the data directly via the class object (public)
  # For example:
  #   data = CSVIO('test.csv')
  #   print data['temp']
  #
  def __getitem__(self, key):

    # Attempt to access the data, produce an error to the screen if it fails
    try:
      return self._data[key]

    except KeyError:
      self.peacockError('No data for key \'' + key + '\' located', dialog=False)
      return None

# tests (see PeacockTestInterface)
  ## Test reading data
  def _testDataRead(self):
    result = data['temp'] == [1.2, 3.3, 5.5]
    fail_msg = 'Data read failed'
    return (result, fail_msg)

  ## Test data access failure
  def _testDataError(self):
    data['ThisDoesNotExist']
    result = self.testLastErrorMessage('No data for key \'ThisDoesNotExist\' located')
    fail_msg = 'Error failed to produce'
    return (result, fail_msg)

  ## Test invalid file input
  def _testInvalidInput(self):
    io = CSVIO('bad_filename.csv', testing=True)
    result = io.testLastErrorMessage('The file \'bad_filename.csv\' does not exist.')
    fail_msg = 'The fake filename exists'
    return (result, fail_msg)


# Perform testing
if __name__ == "__main__":
  data = CSVIO('test.csv', testing=True)
  data.test()
