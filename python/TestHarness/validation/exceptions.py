#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

class ValidationDataKeyAlreadyExists(Exception):
    """
    Exception for when a data key already exists
    """
    def __init__(self, key: str):
        self.key: str = key
        super().__init__(f'Data "{self.key}" is already registered')

class ValidationNoTestsDefined(Exception):
    """
    Exception for when no tests were defined
    """
    def __init__(self, obj):
        super().__init__(f'No test functions defined in {obj.__class__.__name__}')

class ValidationTestMissingResults(Exception):
    """
    Exception for when a test was ran without any results
    """
    def __init__(self, obj, function):
        super().__init__(f'No results reported in {obj.__class__.__name__}.{function}')

class ValidationTestRunException(Exception):
    """
    Exception for when an exception was found when running a test
    """
    def __init__(self, exceptions: list[Exception]):
        super().__init__('Encountered exception(s) while running tests')
        self.exceptions = exceptions
