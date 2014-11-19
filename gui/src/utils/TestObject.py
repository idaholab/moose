from colorText import colorText

##
# A a class for performing testing
#
# Any class that inherits from this interface has a test() method that
# calls any function that is registered using 'registerTest' method.
# The function must output the test result (True | False) and a message
# that will show when the test fails
class TestObject(object):

  ## Constructor (public)
  def __init__(self, **kwargs):

    # Extract QApplication
    self._app = kwargs.pop('app', None)

    # Build a list of tests to perform
    self.tests = []
    for func in dir(self):
      if func.startswith('test'):
        attr = getattr(self, func)
        if hasattr(attr, '__call__'):
          self.tests.append({'name':self.__class__.__name__+'/'+func, 'attr':attr})
          print func
