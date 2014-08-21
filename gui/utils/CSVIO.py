#!/usr/bin/python

from PySide import QtCore, QtGui
import csv


class PeacockErrorInterface:
  def __init__(self, **kwargs):

    self._testing = kwargs.pop('testing', False)
    self._last_error_message = None
    self._error_dialog = None
    self._has_dialog = False

    if isinstance(self, QtGui.QWidget):
      self._error_dialog = QtGui.QErrorMessage(self)
      self._has_dialog = True



  def peacockError(self, *args, **kwargs):

    message = ' '.join(args)
    self._last_error_message = message

    # Eat the message if running in testing mode
    if self._testing:
      return

    if self._has_dialog and kwargs.pop('dialog', True):
      self._error_message.showMessage(message)

    if kwargs.pop('screen', True):
      print message


  def getLastErrorMessage(self):
    return self._last_error_message

class PeacockTestInterface:
  def __init__(self, **kwargs):
    pass

  def test(self):

    class_name = self.__class__.__name__
    prefix = '_test'
    for item in dir(self):
      name = None
      if item.startswith(prefix):
        name = item.replace(prefix, '')
        attr = getattr(self, prefix+name)
        args = attr.func_code.co_argcount
        if args == 1:
          self._showResult(name, attr())
        elif args == 0:
          self._showResult(name, attr)
        else:
          self._showResult(name, False, prefix+name + ' cannot accept arguments')
          return


  def _showResult(self, name, result, *args):
    if result:
      result = 'OK'
    else:
      result = 'FAIL'
      if len(args) == 1 and isinstance(args[0], str) and len(args[0]) > 0 :
        result += ' (' + args[0] + ')'



    name = self.__class__.__name__ + '/' + name
    n = 110 - len(name) - len(result)
    print name + '.'*n + result








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

  def _testDataRead():
    data = CSVIO('test.csv', testing=True)
    return data['data1'] == [1.1, 4.2, 431.353]

  def _testDataError(self):
    data['ThisDoesNotExist']
    return self.getLastErrorMessage() == 'No data for key \'ThisDoesNotExist\' located'

  def _testNope(self, input):
    print input


# Perform testing
if __name__ == "__main__":

  data = CSVIO('test.csv', testing=True)
  data.test()
