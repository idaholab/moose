import re
import os
import logging
log = logging.getLogger(__name__)

from markdown.inlinepatterns import Pattern
from markdown.util import etree
from MooseCommonExtension import MooseCommonExtension
import utils
import MooseDocs

class MooseSyntaxBase(MooseCommonExtension, Pattern):
  """
  Base for MOOSE system/object pattern matching.

  Args:
    regex[str]: The regular expression to match.
    yaml[MooseYaml]: The MooseYaml object for the application.
    syntax[dict]: A dictionary of MooseApplicatinSyntax objects.
  """

  def __init__(self, regex, markdown_instance=None, yaml=None, syntax=None, **kwargs):
    MooseCommonExtension.__init__(self, **kwargs)
    Pattern.__init__(self, regex, markdown_instance)

    self._yaml = yaml
    self._syntax = syntax

    # Error if the YAML was not supplied
    if not isinstance(self._yaml, utils.MooseYaml):
      log.error("The MooseYaml object must be supplied to constructor of MooseObjectClassDescription.")

    # Error if the syntax was not supplied
    if not isinstance(self._syntax, dict):
      log.error("A dictionary of MooseApplicationSyntax objects must be supplied.")
