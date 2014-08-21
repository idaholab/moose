#!/usr/bin/python
import os, sys, csv

# Peacock modules
sys.path.append(os.path.abspath('..'))
from base import *


class CSVIO(object, PeacockErrorInterface, PeacockTestInterface):
  def __init__(self, filename, **kwargs):
    PeacockErrorInterface.__init__(self, **kwargs)
    PeacockTestInterface.__init__(self, **kwargs)


    # Initialize member variables
    self._headers = []
    self._data = dict()

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

  def __getitem__(self, key):

    try:
      return self._data[key]

    except KeyError:
      self.peacockError('No data for key \'' + key + '\' located', dialog=False)
      return None

  def _testDataRead(self):
    result = data['temp'] == [1.2, 3.3, 5.5]
    fail_msg = 'Data read failed'
    return (result, fail_msg)

  def _testDataError(self):
    data['ThisDoesNotExist']
    result = self.getLastErrorMessage() == 'No data for key \'ThisDoesNotExist\' located'
    fail_msg = 'Error failed to produce'
    return (result, fail_msg)

# Perform testing
if __name__ == "__main__":
  data = CSVIO('test.csv', testing=True)
  data.test()
