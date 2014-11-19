from colorText import colorText

##
# A base class for creating test objects
#
# Any method beginning with "test" will be called automatically by PeacockTester
# object. These methods should return a tuple containing the test result (True | False) and
# a message to print in the result was False.
class PeacockTestObject(object):

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
          self.tests.append({'name':self.__class__.__name__+'.'+func, 'attr':attr})
