#For future compatibility with Python 3
from __future__ import division, print_function, unicode_literals, absolute_import
import warnings
warnings.simplefilter('default',DeprecationWarning)

import sys

def find_distribution1D():
  """ find the crow distribution1D module and return it. """
  if sys.version_info.major > 2:
    import crow_modules.distribution1Dpy3
    return crow_modules.distribution1Dpy3
  else:
    import crow_modules.distribution1Dpy2
    return crow_modules.distribution1Dpy2

def find_interpolationND():
  """ find the crow interpolationND module and return it. """
  if sys.version_info.major > 2:
    import crow_modules.interpolationNDpy3
    return crow_modules.interpolationNDpy3
  else:
    import crow_modules.interpolationNDpy2
    return crow_modules.interpolationNDpy2

def checkAnswer(comment,value,expected,results,tol=1e-10):
  """ Will check if a test passes or fails and update the results dictionary.
    @ In, comment: A user-specified comment that will be printed with the test
                   case.
    @ In, value: the generated test value.
    @ In, expected: the gold standard to which value will be compared.
    @ InOut, results: a dictionary to which pass or fail will be incremented.
    @ In, tol: an optional tolerance value specifying how close expected and
               value should be.
  """
  if abs(value - expected) > tol:
    print("checking answer",comment,value,"!=",expected)
    results["fail"] += 1
  else:
    results["pass"] += 1
