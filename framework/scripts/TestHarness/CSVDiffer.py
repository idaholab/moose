import os, re

class CSVDiffer:
  def __init__(self, test_dir, out_files, abs_zero=1e-11, relative_error=5.5e-6):
    self.abs_zero = float(abs_zero)
    self.rel_tol = float(relative_error)
    self.files = []
    self.msg = ''
    self.num_errors = 0

    # read in files
    for out_file in out_files:
      # Check for an easy mistake to make
      if out_file[-2:] == '.e':
        self.msg += 'Diffing ' + out_file + '\n'
        self.msg += 'WARNING: Are you sure you want to use csv diff on a .e file?\n'

      test_filename = os.path.join(test_dir,out_file)
      gold_filename = os.path.join(test_dir, 'gold', out_file)
      if not os.path.exists(test_filename):
        self.addError(test_filename, 'File does not exist!')
      elif not os.path.exists(gold_filename):
        self.addError(gold_filename, 'Gold file does not exist!')
      else:
        try:
          f1 = open( test_filename )
          f2 = open( gold_filename )
          self.addCSVPair(out_file, f1.read(), f2.read())
        except Exception as e:
          self.addError(out_file, 'Exception reading files '+str(e.args))


  # add the contents of two files to be diffed
  def addCSVPair(self, fname, text1, text2):
    self.files.append( (fname, text1, text2) )

  def clearDiff(self):
    """Clears the diff message and errors so diff can be called again"""
    self.msg = ''
    self.num_errors = 0

  # diff the files added to the system and return a message of differences
  # This method should only be called once. If it called again you must
  # manually clear messages by calling clearDiff
  def diff(self):

    for fname, text1, text2 in self.files:
      # use this value to skip the rest of the tests when we've found an error
      # the order of the tests is most general to most specific, so if a general
      # one fails then the more specific ones will probably not only fail, but
      # crash the program because it's looking in a column that doesn't exist
      foundError = False

      table1 = self.convertToTable(fname, text1)
      table2 = self.convertToTable(fname, text2)

      # Make sure header names are the same (also makes sure # cols is the same)
      # This way it reports what column is missing, not just # cols is different
      keys1 = table1.keys()
      keys2 = table2.keys()
      (large,small) = (keys1,keys2)
      if len(keys1) < len(keys2):
        (large,small) = (keys2,keys1)
      for key in large:
        if key not in small:
          self.addError(fname, "Header '" + key + "' is missing" )
          foundError = True
      if foundError:
        continue

      # now check that each column is the same length
      for key in keys1:
        if len(table1[key]) != len(table2[key]):
          self.addError(fname, "Columns with header '" + key + "' aren't the same length")
          foundError = True
          # assume all columns are the same length, so don't report the other errors
          break
      if foundError:
        continue

      # now check all the values in the table
      abs_zero = self.abs_zero
      rel_tol   = self.rel_tol
      for key in keys1:
        for val1, val2 in zip( table1[key], table2[key] ):
          # adjust to the absolute zero
          if abs(val1) < abs_zero:
            val1 = 0
          if abs(val2) < abs_zero:
            val2 = 0

          # if they're both exactly zero (due to the threshold above) then they're equal so pass this test
          if val1 == 0 and val2 == 0:
            continue

          rel_diff = abs( ( val1 - val2 ) / max( abs(val1), abs(val2) ) )
          if rel_diff > rel_tol:
            self.addError(fname, "The values in column " + key + " don't match")
            # assume all other vals in this column are wrong too, so don't report them
            break

    return self.msg


  # convert text to a map of column names to column values
  def convertToTable(self, fname, text):
    # ignore newlines
    text = re.sub( r'\n\s*\n', '\n', text).strip()

    # Exceptions occur if you try to parse a .e file
    try:
      lines = text.split('\n')
      headers = lines.pop(0).split(',')
      table = {}
      for header in headers:
        table[header] = []

      for row in lines:
        vals = row.split(',')
        if len(headers) != len(vals):
          self.addError(fname, "Number of columns ("+str(len(vals))+") not the same as number of column names ("+str(len(headers))+") in row "+repr(row))
        for header, val in zip(headers,vals):
          table[header].append(float(val))

    except Exception as e:
      self.addError(fname, "Exception parsing file: "+str(e.args))
      return {}

    return table

  # add an error to the message
  # every error is added through here, so it could also output in xml, etc.
  def addError(self, fname, message):
    self.msg += 'In ' + fname + ': ' + message + '\n'
    self.num_errors += 1

  def getNumErrors(self):
    """Return number of errors in diff"""
    return self.num_errors


# testing the test harness!
if __name__ == '__main__':
  # Test for success and ignoring newlines
  d = CSVDiffer(None, [])
  d.addCSVPair('out.csv', 'col1,col2\n1,2\n1,2', 'col1,col2\n1,2\n1,2')
  d.addCSVPair('out2.csv', 'col1,col2\n \n1,2\n1,2', 'col1,col2\n1,2\n\t\n1,2')
  d.addCSVPair('out3.csv', 'col1,col2\n1,2\n1,2\n\n', 'col1,col2\n1,2\n1,2')
  print 'Should be 0 errors'
  print d.diff()

  # Test for different number of columns
  d = CSVDiffer(None, [])
  d.addCSVPair('out.csv', 'col1,col2\n1,2\n1,2', 'col1,col2,col3\n1,2,3\n1,2,3')
  print 'Should be 1 error'
  print d.diff()

  # Test for different column lengths
  d = CSVDiffer(None, [])
  d.addCSVPair('out1.csv', 'col1,col2\n1,2\n1,2', 'col1,col2,col3\n1,2,3\n1,2,3')
  d.addCSVPair('out2.csv', 'col1,col2\n1,2\n1,2\n3,4', 'col1,col2\n1,2\n1,2')
  print 'Should be 2 errors'
  print d.diff()

  # Test for absolute zero logic
  d = CSVDiffer(None, [])
  d.addCSVPair('out1.csv', 'col1,col2\n1,2\n1,2', 'col1,col2\n1,2\n1,2.1')
  d.addCSVPair('out2.csv', 'col1,col2\n1,2\n1,2\n0,-1e-13', 'col1,col2\n1,2\n1,2\n1e-12,1e-13')
  d.addCSVPair('out3.csv', 'col1,col2\n1,2\n1,2\n0,0', 'col1,col2\n1,2\n1,2\n1e-4,1e-13')
  print 'Should be 2 errors'
  print d.diff()

  # Test relative tolerance
  d = CSVDiffer(None, [])
  d.addCSVPair('out1.csv', 'col1,col2\n1,2\n1,2', 'col1,col2\n1,2\n1,2.1')
  d.addCSVPair('out2.csv', 'col1,col2\n1,-2\n1,-2', 'col1,col2\n1,-2\n1,-2.00000000001')
  d.addCSVPair('out3.csv', 'col1,col2\n1,2\n1,2', 'col1,col2\n1,2\n1.00001,2.0001')
  print 'Should be 3 errors'
  print d.diff()

  # test file does not exist
  d = CSVDiffer('qwertyuiop', ['out.csv'])
  print 'Should be 1 error'
  print d.diff()
