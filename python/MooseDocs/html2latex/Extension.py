#pylint: disable=missing-docstring
####################################################################################################
#                                    DO NOT MODIFY THIS HEADER                                     #
#                   MOOSE - Multiphysics Object Oriented Simulation Environment                    #
#                                                                                                  #
#                              (c) 2010 Battelle Energy Alliance, LLC                              #
#                                       ALL RIGHTS RESERVED                                        #
#                                                                                                  #
#                            Prepared by Battelle Energy Alliance, LLC                             #
#                               Under Contract No. DE-AC07-05ID14517                               #
#                               With the U. S. Department of Energy                                #
#                                                                                                  #
#                               See COPYRIGHT for full restrictions                                #
####################################################################################################
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
