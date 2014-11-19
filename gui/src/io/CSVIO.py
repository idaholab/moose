#!/usr/bin/python
import os, sys, csv

# Peacock modules
from src.base import *

##
# A class for performing read/write operations on *.csv files
class CSVIO(PeacockErrorInterface):

  ##
  # Class constructor
  # @param filename The name of the csv file top open
  # @param kwargs Optional keyword/value pairs
  #
  # Optional Arguments:
  #   @see PeacockErrorInterface, PeacockTestInterface
  def __init__(self, filename, **kwargs):
    PeacockErrorInterface.__init__(self, **kwargs)

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
