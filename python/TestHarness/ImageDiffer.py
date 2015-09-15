#!/usr/bin/env python
import os
from pylab import imread
import numpy

##
# Storage container for error and detailed message
class ImageError(object):

  ##
  # Constructor.
  # @param err A one line error message
  # @param msg A detailed message describing the error
  def __init__(self, err, msg):
    self.error = err
    self.message = msg


##
# A class for comparing image files
class ImageDiffer(object):

  def __init__(self, file1, file2, **kwargs):

    # Store the file names
    self.__files = [file1, file2]

    # Extract the optional arguments
    self.__abs_zero = float(kwargs.pop('abs_zero', 1e-11))

    # Storage for error messages
    self.__errors = []

    # Read the image files
    self.__data = []
    self.__data.append(self.__readImage(self.__files[0]))
    self.__data.append(self.__readImage(self.__files[1]))

    # Perform comparison
    self.__compare()


  ##
  # Check the comparison status (public)
  # Returns True if the comparison fails
  def fail(self):
    return len(self.__errors) > 0

  ##
  # Print the error message(s) (public)
  # @return The output as a single string
  def message(self, **kwargs):

    # Header
    output = []
    output.append('Running ImageDiffer.py')
    output.append( '         File 1: ' + self.__files[0])
    output.append( '         File 2: ' + self.__files[1])
    output.append( '       abz_zero: ' + str(self.__abs_zero))
    output.append( '  No. of errors: ' + str(len(self.__errors)))

    # Errors
    cnt = 0
    for e in self.__errors:
        cnt += 1
        output.append('\n')
        output.append('ERROR ' + str(cnt) + ':')
        output.append('  ' + e.error)
        for m in e.message:
            output.append('    ' + m)

    # Print the output
    if kwargs.pop('output', False):
        print '\n'.join(output)

    # Return the text, as a single string
    return '\n'.join(output)


  ## Perform image comparison
  #
  # The images are compared by computing the norm of the difference
  # of the images via numpy.
  def __compare(self):

    # Do nothing if something failed to open
    for d in self.__data:
      if d.shape == (0):
        return

    # Check sizes
    if (self.__data[0].shape != self.__data[1].shape):
      err = 'The two images are different sizes'
      msg  =  ['  File 1: ' + self.__files[0]]
      msg +=  ['    size: ' + str(self.__data[0].shape)]
      msg +=  ['  File 2: ' + self.__files[1]]
      msg +=  ['    size: ' + str(self.__data[1].shape)]
      self.__addError(err, msg)
      return

    # Compute Norm
    diff = numpy.linalg.norm(self.__data[0] - self.__data[1])

    if diff > self.__abs_zero:
      err = 'The files are different.'
      msg = ['The norm of the difference of the images exceeds "abs_zero"']
      msg += ['    ||file1 - file2|| = ' + str(diff)]
      msg += ['             abz_zero = ' + str(self.__abs_zero)]
      self.__addError(err, msg)
      return


  ##
  # Read image
  def __readImage(self, filename):

    # Check for file existence
    if not os.path.isfile(filename):
      self.__addError('Could not open ' + filename + ', the file does not exist.')
      return numpy.empty([0])

    # Read the array
    try:
      data = imread(filename)
      return data
    except Exception, e:
      print e
      self.__addError('Could not read image ' + filename + ' using numpy.imread.')


  ##
  # Add an ImageError object to the storage vector (private)
  # @param err A string containing the error message or an XMLError object
  # @param msg A detailed message for the error (ignored if XMLError is passed to err)
  def __addError(self, err, msg=[]):

    # Add object directly
    if isinstance(err, ImageError):
      self.__errors.append(err)

    # Create and add the object
    else:
      obj = ImageError(err, msg)
      self.__errors.append(obj)

# This file is executable and performs tests of hte functionality
if __name__ == '__main__':

  # Test files
  file0 = os.path.join(os.getenv('MOOSE_DIR'), 'python', 'TestHarness', 'test_files', 'andrew.png')
  file1 = os.path.join(os.getenv('MOOSE_DIR'), 'python', 'TestHarness', 'test_files', 'andrew2.png')
  file2 = os.path.join(os.getenv('MOOSE_DIR'), 'python', 'TestHarness', 'test_files', 'john.png')

  # Lines for making things look pretty
  lines = '-'*129

  # Test invalid filenames (1 error)
  print lines
  print 'Test 1: Invalid filename (1 error)'
  print lines
  d = ImageDiffer(file0, 'not_a_file.jpeg')
  if d.fail():
    d.message(output=True)

  # Test different size images (1 error)
  print lines
  print 'Test 2: Different size image (1 error)'
  print lines
  d = ImageDiffer(file0, file1)
  if d.fail():
    d.message(output=True)

  # Test different images of same size (1 error)
  print lines
  print 'Test 3: Different images of same size (1 error)'
  print lines
  d = ImageDiffer(file0, file2)
  if d.fail():
    d.message(output=True)

  # Test same images
  print lines
  print 'Test 4: Same images'
  print lines
  d = ImageDiffer(file0, file0)
  if not d.fail():
    print 'The files are the same\n'
