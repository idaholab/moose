import os, sys

# Import Peacock IO module
from src import io
from src.utils import TestObject


class CSVIOTest(TestObject):
  def __init__(self, **kwargs):
    TestObject.__init__(self, **kwargs)

    # Set global test filename
    self.test_file = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'input.csv')

  # Test reading
  def testDataRead(self):
    csvio = io.CSVIO(self.test_file, testing=True)
    result = csvio['temp'] == [1.2, 3.3, 5.5]
    fail_msg = 'Data read failed'
    return (result, fail_msg)

  # Test data access failure
  def testDataError(self):
    csvio = io.CSVIO(self.test_file, testing=True)
    csvio['ThisDoesNotExist']
    result = csvio.testLastErrorMessage('No data for key \'ThisDoesNotExist\' located')
    fail_msg = 'Error failed to produce'
    return [result, fail_msg]

  # Test invalid file input
  def testInvalidInput(self):
    csvio = io.CSVIO('bad_filename.csv', testing=True)
    result = csvio.testLastErrorMessage('The file \'bad_filename.csv\' does not exist.')
    fail_msg = 'The fake filename exists'
    return [result, fail_msg]
