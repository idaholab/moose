#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os, traceback
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
    #   abs_zero: Any value less than this is assumed zero (default: 1e-11)
    #   rel_tol: Relative tolerance to check numeric values against (default: 5.5e-6)
    #   max_values: The maximum number of values to test
    def __init__(self, file1, file2, **kwargs):

        # Store the file names
        self._file = [file1, file2]

        # Extract the optional arguments
        self._abs_zero = float(kwargs.pop('abs_zero', 1e-11))
        self._rtol = float(kwargs.pop('rel_tol', 5.5e-6))
        self._ignored_attributes = kwargs.pop('ignored_attributes', [])

        # Storage for XMLError objects
        self._errors = []

        # Extract the XML tree from the files
        self._root1 = self._extractXML(file1)
        self._root2 = self._extractXML(file2)

        # Perform the comparison
        self._compare()

    ##
    # Check the comparison status (public)
    # Returns True if the comparison fails
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
        output.append( '       abs_zero: ' + str(self._abs_zero))
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
            print('\n'.join(output))

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
            err = 'An XML parser error occurred attempting to read XML tree from ' + filename + '.'
            msg = traceback.format_exc().splitlines()
            self._addError(err, msg)
            root = None

        # Return the object
        return root

    ##
    # Perform the block by block comparison (private)
    def _compare(self):

        # Define local variables
        root = [self._root1, self._root2]

        # Do nothing if the objects do not exist
        if root[0] == None or root[1] == None:
            return

        # Loop through each tree object in the master file
        for elem0 in root[0].getiterator():

            # Initialize the result and error storage
            results = []
            errors  = []

            # Loop through all blocks in the second file with the current tag
            for elem1 in root[1].getiterator(elem0.tag):

                # Perform the comparison
                r, e = self._compareBlock(elem0, elem1)

                # Append the test results
                results.append(r)
                errors.append(e)

            # If all results are False, there was no match
            if not any(results):

                # Filter out errors (elem.text failure)
                errors = [_f for _f in errors if _f]

                # If no errors exist there was no block or block with identical attributes located
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
    # @return A pair containing the test result (True or False) and an error indicator,
    #         this 'indicator' is None if the result of the match is True or if the
    #         attributes fail to match. When the text fails to match then it contains
    #         the XMLError object.
    def _compareBlock(self, elem0, elem1):

        # Perform attribute comparison in both directions: ensure that
        # every attribute in the gold file is in the output file, and
        # vice-versa.
        test_attrib = self._compareAttributes(elem0, elem1) and self._compareAttributes(elem1, elem0)

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
        for key0, value0 in elem0.attrib.items():

            # If this key is one of the attributes we're ignoring, then ignore it!
            if key0 in self._ignored_attributes:
                continue

            # Attribute is missing from the slave object, match fails
            if key0 not in elem1.attrib:
                return  False

            # If the slave object has the same attribute, perform a comparison
            elif key0 in elem1.attrib:
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
        text0 = [_f for _f in text0 if _f]
        text1 = [_f for _f in text1 if _f]

        # Check that the lengths are the same
        if len(text0) != len(text1):
            result = False
            err = 'An XML block with the tag "' + elem0.tag + '" and the following attributes exists in both files, but the blocks have a different number of values.'
            msg = self._getAttrib(elem0)
            msg.append('No. items file 1: ' + '%d' % len(text0))
            msg.append('No. items file 2: ' + '%d' % len(text1))
            err = XMLError(err, msg)
            return (False, err)

        for i in range(len(text0)):
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

        # Return the comparison
        return result, rel_diff


    ##
    # Get the attributes (dict) as a string (private)
    # @return Attribute message string
    def _getAttrib(self, elem):
        msg = []
        for k, v in elem.attrib.items():
            msg.append('  ' + k + ' = ' + v)
        return msg


if __name__ == '__main__':
    # You can run XMLDiffer.py as a stand-alone by putting two XML files
    # in the variable names file1 and file2 below, and then running:
    #
    # python $MOOSE_DIR/python/TestHarness/XMLDiffer.py
    file1 = os.path.join(os.getenv('MOOSE_DIR'), 'test', 'tests', 'outputs', 'vtk', 'vtk_diff_serial_mesh_parallel_out_005.pvtu')
    file2 = os.path.join(os.getenv('MOOSE_DIR'), 'test', 'tests', 'outputs', 'vtk', 'gold', 'vtk_diff_serial_mesh_parallel_out_005.pvtu')

    d = XMLDiffer(file1, file2, ignored_attributes=['header_type'])
    if not d.fail():
        print('Files are the same\n')
    else:
        print(d.message())
