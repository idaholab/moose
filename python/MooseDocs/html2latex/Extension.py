class Extension(object):
    """
    Base class for creating extensions.

    Args:
      kwargs[dict]: All key, value pairings are stored as "configuration" options, see getConfigs.
    """
    def __init__(self, **kwargs):

        #: Configure options
        self._configs = kwargs
        self._configs.setdefault('headings', ['section', 'subsection', 'subsubsection', 'textbf', 'underline', 'emph'])

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
