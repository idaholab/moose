#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
#pylint: enable=missing-docstring

class Extension(object):
    """
    Base class for creating extensions.

    Args:
      kwargs[dict]: All key, value pairings are stored as "configuration" options, see getConfigs.
    """
    def __init__(self, **kwargs):

        #: Configure options
        self._configs = kwargs
        headings = ['section', 'subsection', 'subsubsection', 'textbf', 'underline', 'emph']
        self._configs.setdefault('headings', headings)

    def getConfigs(self):
        """
        Return the dictionary of configure options.
        """
        return self._configs

    def extend(self, translator):
        """
        Elements should be added to the storage of the Translator instance within this function.

        Args:
          translator[Translator]: The object to be used for converting the html.
        """
        pass
