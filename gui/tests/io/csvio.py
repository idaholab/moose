#!/usr/bin/python
import os, sys

# Import Peacock IO module
from src import io

# Set global test filename
test_file = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'input.csv')

# Test reading
def testDataRead():
  csvio = io.CSVIO(test_file)
  result = csvio['temp'] == [1.2, 3.3, 5.5]
  fail_msg = 'Data read failed'
  return (result, fail_msg)

# Test data access failure
def testDataError():
  csvio = io.CSVIO(test_file)
  csvio['ThisDoesNotExist']
  result = csvio.testLastErrorMessage('No data for key \'ThisDoesNotExist\' located')
  fail_msg = 'Error failed to produce'
  return [result, fail_msg]

# Test invalid file input
def testInvalidInput():
  csvio = io.CSVIO('bad_filename.csv')
  result = csvio.testLastErrorMessage('The file \'bad_filename.csv\' does not exist.')
  fail_msg = 'The fake filename exists'
  return [result, fail_msg]
