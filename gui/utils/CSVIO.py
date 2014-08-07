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
  def __init__(self):
    pass

  def test(self):

    class_name = self.__class__.__name__
    static_prefix = '_' + class_name + '__test'
    prefix = '__test'

    print dir(self)

    for item in dir(self):
      name = None

      print item

      # static
      if item.startswith(static_prefix):
        print "Static"
        name = item.replace(static_prefix, '')
        attr = getattr(self, item)
        self.__showResult(name, attr)


      if item.startswith(prefix):
        print "Non-static"
        name = item.replace(prefix, '')
        attr = getattr(self, '__test'+name)
        result = attr()
        self.__showResult(name, result)

  def __showResult(self, name, result):

    print 'result = ', result

    if result:
      result = 'OK'
    else:
      result = 'FAIL'


    name = self.__class__.__name__ + '/' + name
    n = 110 - len(name) - len(result)
    print name + '.'*n + result








class CSVIO(object, PeacockErrorInterface, PeacockTestInterface):
  def __init__(self, filename, **kwargs):
    PeacockErrorInterface.__init__(self, **kwargs)
    PeacockTestInterface.__init__(self)


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


  def _testDataAccess():
    data = CSVIO('test.csv', testing=True)
    return data['data1'] == [1.1, 4.2, 431.353]

  def _testDataError(self):
    data['ThisDoeNotExist']
    print self.getLastErrorMessage()
    return self.getLastErrorMessage() == 'No data for key \'ThisDoeNotExist\' loated'


# Perform testing
if __name__ == "__main__":

  data = CSVIO('test.csv', testing=True)
  data.test()

  print data.getLastErrorMessage()
