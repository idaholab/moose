import re
import os
import logging
log = logging.getLogger(__name__)

from markdown.util import etree
from MooseSyntaxBase import MooseSyntaxBase
import utils
import MooseDocs

class MooseActionSyntax(MooseSyntaxBase):
  """
  Creates tables of sub-object/systems.

  !subobjects = A list of subobjects /* and /<type>
  !subsystems = A list of sub-syntax (e.g, Markers and Indicators)

  Available Settings:
    title[str]: Set the title of the section defined above the table.
  """

  RE = r'^!(subobjects|subsystems)\s+(.*?)(?:$|\s+)(.*)'

  def __init__(self, yaml=None, syntax=None, **kwargs):
    MooseSyntaxBase.__init__(self, self.RE, yaml=yaml, syntax=syntax, **kwargs)
    self._settings['title'] = None

  def handleMatch(self, match):
    """
    Handle the regex match.
    """
    # Extract match options and settings
    action = match.group(2)
    syntax = match.group(3)
    settings = self.getSettings(match.group(4))

    if action == 'subobjects':
      el = self.subobjectsElement(syntax, settings)
    elif action == 'subsystems':
      el = self.subsystemsElement(syntax, settings)
    return el

  def subobjectsElement(self, sys_name, settings):
    """
    Create table of sub-objects.
    """
    collection = MooseDocs.extensions.create_object_collection(sys_name, self._syntax)
    if collection:
      el = self.applyElementSettings(etree.Element('div'), settings)
      h2 = etree.SubElement(el, 'h2')
      h2.text = settings['title'] if settings['title'] else 'Available Sub-Objects'
      el.append(collection)
    else:
      el = etree.Element('p')
    return el

  def subsystemsElement(self, sys_name, settings):
    """
    Create table of sub-systems.
    """
    collection = MooseDocs.extensions.create_system_collection(sys_name, self._syntax)
    if collection:
      el = self.applyElementSettings(etree.Element('div'), settings)
      h2 = etree.SubElement(el, 'h2')
      h2.text = settings['title'] if settings['title'] else 'Available Sub-Systems'
      el.append(collection)
    else:
      el = etree.Element('p')
    return el
