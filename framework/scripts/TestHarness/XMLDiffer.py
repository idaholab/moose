import os, sys, traceback, numpy
import xml.etree.ElementTree as xml

##
# Stores error information needed for printing diff messages
class XMLError(object):

  ##
  # Constructor.
  # @param err A one line error message
  # @param msg A detailed message describing the error
  def __init__(self, err, msg):
    self.error = err
    self.message = msg

##
# A class for finding difference between XML documents
class XMLDiffer(object):

  ##
  # Constructor.
  # @param file1 The master (gold) file to check against
  # @param file2 The file to compare to the master file
  #
  # Optional Arguments:
  #   abz_zero Any value less than this is assumed zero (default: 1e-11)
  #   rel_tol Relative tolerance to check numeric values against (default: 5.5e-6)
  #   max_values The maximum number of values to test
  def __init__(self, file1, file2, **kwargs):

    # Store the file names
    self._file = [file1, file2]

    # Extract the optional arguments
    self._abs_zero = float(kwargs.pop('abs_zero', 1e-11))
    self._rtol = float(kwargs.pop('rel_tol', 5.5e-6))

    # Storage for XMLError objects
    self._errors = []

    # Extract the XML tree from the files
    self._root1 = self._extractXML(file1)
    self._root2 = self._extractXML(file2)

    # Perform the comparision
    self._compare()

  ##
  # Check the comparision status (public)
  # Returns True if the comparision fails
  def fail(self):
    return len(self._errors) > 0

  ##
  # Print the error message(s) (public)
  # @return The output as a single string
  def message(self, **kwargs):

    # Header
    output = []
    output.append('Running XMLDiffer.py')
    output.append( '         File 1: ' + self._file[0])
    output.append( '         File 2: ' + self._file[1])
    output.append( '        rel_tol: ' + str(self._rtol))
    output.append( '       abz_zero: ' + str(self._abs_zero))
    output.append( '  No. of errors: ' + str(len(self._errors)))

    # Errors
    cnt = 0
    for e in self._errors:
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

  ##
  # Add an XMLError object to the storage vector (private)
  # @param err A string containing the error message or an XMLError object
  # @param msg A detailed message for the error (ignored if XMLError is passed to err)
  def _addError(self, err, msg=[]):

    # Add object directly
    if isinstance(err, XMLError):
      self._errors.append(err)

    # Create and add the object
    else:
      obj = XMLError(err, msg)
      self._errors.append(obj)

  ##
  # Reads the XML file (private)
  # @param filename The name of the XML file to read
  # @return An xml.etree.ElementTree root object
  def _extractXML(self, filename):

    # Check for file existence
    if not os.path.isfile(filename):
      self._addError('Could not open ' + filename + ', the file does not exist.')
      return None

    # Get the root object of the XML tree
    try:
      root = xml.parse(filename).getroot()

    # Catch parser errors
    except xml.ParseError:
      err = 'An XML parser error occured attemptting to read XML tree from ' + filename + '.'
      msg = traceback.format_exc().splitlines()
      self._addError(err, msg)
      root = None

    # Return the object
    return root

  ##
  # Perform the block by block comparision (private)
  def _compare(self):

    # Define local variables
    root = [self._root1, self._root2]
    name = ['file 1', 'file 2']

    # Do nothing if the objects do not exist
    if root[0] == None or root[1] == None:
      return

    # Loop through each tree object in the master file
    for elem0 in root[0].iter():

      # Intialize the result and error storage
      results = []
      errors  = []

      # Loop through all blocks in the second file with the current tag
      for elem1 in root[1].iter(elem0.tag):

        # Perform the comparision
        r, e = self._compareBlock(elem0, elem1)

        # Append the test results
        results.append(r)
        errors.append(e)

      # If all results are False, there was no match
      if not any(results):

        # Fiter out errors (elem.text failure)
        errors = filter(None, errors)

        # If no erros exist there was no block or block with identical attributes located
        if len(errors) == 0:
          msg = self._getAttrib(elem0)
          if len(msg) == 0:
            err = 'Unable to locate an XML Block with the tag "' + elem0.tag + '" in file 2.'
            self._addError(err)
          else:
            err = 'Unable to locate an XML Block with the tag "' + elem0.tag + '" and the following attributes in file 2.'
            self._addError(err, msg)

        # Had a text error within similar blocks
        else:
          for e in errors:
            self._addError(e)


  ##
  # Compares XML blocks (private)
  # This function first compares the XML block attributes, if those match
  # then the XML text is also compared.
  # @param elem0 The master XML element object
  # @param elem1 The XML element object to compare the master against
  # @return A pair containg the test result (True or False) and an error indicator,
  #         this 'indicator' is None if the result of the match is True or if the
  #         attributes fail to match. When the text fails to match then it contains
  #         the XMLError object.
  def _compareBlock(self, elem0, elem1):

    # Perform attribute comparision
    test_attrib = self._compareAttributes(elem0, elem1)

    # If the attributes match, compare the text and return those results
    if test_attrib:
      test_text, err = self._compareText(elem0, elem1)
      return test_text, err

    # Otherwise the attributes do match
    else:
      return False, None

  ##
  # Perform attribute comparison (private)
  # @param elem0 The master XML element object
  # @param elem1 The XML element object to compare the master against
  def _compareAttributes(self, elem0, elem1):

    # Initialize the output (assume match)
    result = True

    # Loop through each attribute of the master object
    for key0, value0 in elem0.attrib.iteritems():

      # Attribute is missing from the slave object, match fails
      if not elem1.attrib.has_key(key0):
        return  False

      # If the slave object has the same attribute, perform a comparison
      elif elem1.attrib.has_key(key0):
        value1 = elem1.attrib[key0]

        # Attempt to perform a numeric comparison
        try:
          tvalue, rel_diff = self._isClose(value0, value1)
          if not tvalue:
            return False

        except:
          if value0 != value1:
            return False

    # Return the results
    return result

  ## Perform comparison of text for two XML blocks (private)
  # @param elem0 The master XML element object
  # @param elem1 The XML element object to compare the master against
  # @return A pair of items, either True, None or False, XMLError
  def _compareText(self, elem0, elem1):

    # Initialize the output
    result = True
    err = None

    # Return if no text exists
    if elem0.text == None and elem1.text == None:
      return (result, err)
    elif elem0.text == None or elem1.text == None:
      return (False, err)

    # Convert the text to a list of strings
    text0 = elem0.text.replace('\n', '').strip().split(' ')
    text1 = elem1.text.replace('\n', '').strip().split(' ')
    text0 = filter(None, text0)
    text1 = filter(None, text1)

    # Check that the lengths are the same
    if len(text0) != len(text1):
      result = False
      err = 'An XML block with the tag "' + elem0.tag + '" and the following attributes exists in both files, but the blocks have a diffenent number of values.'
      msg = self._getAttrib(elem0)
      msg.append('No. items file 1: ' + '%d' % len(text0))
      msg.append('No. items file 2: ' + '%d' % len(text1))
      err = XMLError(err, msg)
      return (False, err)

    for i in xrange(len(text0)):
      value, rel_diff = self._isClose(text0[i], text1[i])

      if not value:
        err = 'An XML block with the tag "' + elem0.tag + '" and the following attributes has differing values on file 2.'
        msg = self._getAttrib(elem0)
        msg.append('Index ' + str(i) + ' : ' + text0[i] + ' ~ ' + text1[i] + ', rel diff: ' + '%e' % rel_diff)
        err = XMLError(err, msg)
        return (False, err)

    return result, err

  ##
  # Perform relative tolerance check between two numbers (private)
  # @param value0 A string or list of strings containing the first number
  # @param value1 A string or list of strings containing the second number
  def _isClose(self, value0, value1):

    # Return values
    result = True
    rel_diff = 0

    # Convert the strings to floats
    value0 = float(value0)
    value1 = float(value1)

    # Apply the absolute zeros
    if abs(value0) < self._abs_zero:
      value0 = 0
    if abs(value1) < self._abs_zero:
      value1 = 0

    # Check for zero
    if value0 == 0 and value1 == 0:
      result = True

    # Check the relative error
    else:
      rel_diff = abs( ( value0 - value1 ) / max( abs(value0), abs(value1) ) )
      if rel_diff > self._rtol:
        result = False

    # Return the comparision
    return result, rel_diff


  ##
  # Get the attributes (dict) as a string (private)
  # @return Attribute message string
  def _getAttrib(self, elem):
    msg = []
    for k, v in elem.attrib.iteritems():
      msg.append('  ' + k + ' = ' + v)
    return msg


if __name__ == '__main__':

  # 'gold' file
  file0 = os.path.join(os.getenv('MOOSE_DIR'), 'framework', 'scripts', 'TestHarness', 'test_files', 'xml_differ_test0.vtu')

  # Has invalid tags
  file1 = os.path.join(os.getenv('MOOSE_DIR'), 'framework', 'scripts', 'TestHarness', 'test_files', 'xml_differ_test1.vtu')

  # Has blocks with different attributes and values (text)
  file2 = os.path.join(os.getenv('MOOSE_DIR'), 'framework', 'scripts', 'TestHarness', 'test_files', 'xml_differ_test2.vtu')

  # Has blocks with tolerance differences
  file3 = os.path.join(os.getenv('MOOSE_DIR'), 'framework', 'scripts', 'TestHarness', 'test_files', 'xml_differ_test3.vtu')

  # Lines for making things look pretty
  lines = '-'*129

  # Test invalid filenames (1 error)
  print lines
  print 'Test 1: Invalid filename (1 error)'
  print lines
  d = XMLDiffer(file0, 'adfadfas.vtk')
  if d.fail():
    d.message(output=True)

  # Parser error (i.e., opening/closing tags do not match)
  print '\n' + lines
  print 'Test 2: Parser Error (1 error)'
  print lines
  d = XMLDiffer(file0, file2)
  if d.fail():
    d.message(output=True)

  # Test missing block and different attribute
  #  (1) Unable locate block (Piece as different attributes in 2)
  #  (2) Differing values in Points/DataArray
  #  (3) No CellData block in 2
  #  (4) Different no. of values in Cells/DataArray (Name = types)
  print '\n' + lines
  print 'Test 3: Block Errors (4 errors)'
  print lines
  d = XMLDiffer(file0, file1)
  if d.fail():
    d.message(output=True)

  # Test abs_zero
  # The first call should not error, the second should diff
  print '\n' + lines
  print 'Test 4: Absolute zero (1 error)'
  print lines
  # This should not error
  d = XMLDiffer(file0, file3, abs_zero=1e-10)
  if not d.fail():
    print 'Successfully set value with lower abs_zero flag\n'

  d = XMLDiffer(file0, file3, abs_zero=1e-12)
  if d.fail():
    d.message(output=True)

  # Test rel_tol
  # The first call should not error, the second should diff
  print '\n' + lines
  print 'Test 5: Relative Tolerance (1 error)'
  print lines
  d = XMLDiffer(file0, file3)
  if not d.fail():
    print 'Successfully checked value within default tolerance\n'

  d = XMLDiffer(file0, file3, rel_tol=1e-10)
  if d.fail():
    d.message(output=True)

  # Test no-error
  print '\n' + lines
  print 'Test 6: Same File (0 error)'
  print lines
  d = XMLDiffer(file0, file0)
  if not d.fail():
    print 'Successfully checked the same file\n'
