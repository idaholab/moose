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
