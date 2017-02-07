import logging
log = logging.getLogger(__name__)

from markdown.inlinepatterns import Pattern
from MooseCommonExtension import MooseCommonExtension

class MooseSyntaxBase(MooseCommonExtension, Pattern):
    """
    Base for MOOSE system/object pattern matching.

    Args:
      regex[str]: The regular expression to match.
      yaml[MooseYaml]: The MooseYaml object for the application.
      syntax[dict]: A dictionary of MooseApplicatinSyntax objects.
    """

    def __init__(self, regex, markdown_instance=None, syntax=None, **kwargs):
        MooseCommonExtension.__init__(self, **kwargs)
        Pattern.__init__(self, regex, markdown_instance)

        self._syntax = syntax

        # Error if the syntax was not supplied
        if not isinstance(self._syntax, dict):
            log.error("A dictionary of MooseApplicationSyntax objects must be supplied.")

    def getInfo(self, name):
        info = self.getObject(name)
        if info == None:
            return self.getAction(name)
        return info

    def getObject(self, name):
        for syntax in self._syntax.itervalues():
            if syntax.hasObject(name):
                return syntax.getObject(name)
        return None

    def getAction(self, name):
        for syntax in self._syntax.itervalues():
            if syntax.hasAction(name):
                return syntax.getAction(name)
        return None
