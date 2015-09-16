import sys

if sys.version_info.major > 2:
  import crow_modules.distribution1Dpy3
  import crow_modules.interpolationNDpy3
else:
  import crow_modules.distribution1Dpy2
  import crow_modules.interpolationNDpy2

sys.exit(0)
