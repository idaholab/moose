import re
import os
import logging
log = logging.getLogger(__name__)

from markdown.util import etree
from MooseSyntaxBase import MooseSyntaxBase
import utils
import MooseDocs

class MooseActionList(MooseSyntaxBase):
  """
  Creates dynamic lists for Moose syntax.

  Examples:
  !systems
  !systems framework phase_field
  """

  RE = r'^!systems\s*(.*)'

  def __init__(self, yaml=None, syntax=None, **kwargs):
    MooseSyntaxBase.__init__(self, self.RE, syntax=syntax, **kwargs)
    self._settings['groups'] = None
    self._yaml = yaml

  def handleMatch(self, match):
    """
    Handle the regex match for this extension.
    """

    # Extract settings
    settings = self.getSettings(match.group(2))

    # Extract the data to consider
    groups = self._syntax.keys()
    if settings['groups']:
      groups = settings['groups'].split()

    # Build complete list of action objects
    actions = []
    for syn in self._syntax.itervalues():
      actions += syn._actions.values()
    actions = sorted(actions, key=lambda x: x.key)

    # Create the primary element
    el = etree.Element('div')
    el.set('class', 'moose-system-list')

    # Loop over keys
    for action in actions:

      if action.hidden:
        continue

      # Create the top-level section
      level = len(action.key.strip('/').split('/'))
      id = action.name.replace(' ', '_').lower()
      if level == 1:
        div = etree.Element('div')
        div.set('class', 'section scrollspy')
        div.set('id', id)

      # Current heading
      h = etree.Element('h{}'.format(str(level+1)))
      h.text = action.name
      h.set('id', id)

      # Add link to action pages
      a = etree.SubElement(h, 'a')
      a.set('href', action.markdown)
      i = etree.SubElement(a, 'i')
      i.set('class', 'material-icons')
      i.text = 'input'

      # Create a chip showing where the action is defined
      tag = etree.SubElement(h, 'div')
      tag.set('class', 'chip moose-chip')
      tag.text = action.group

      # Build a table(s) of sub-objects
      collection = MooseDocs.extensions.create_object_collection(action.key, self._syntax, groups=groups)

      # If a table exists then this section should be displayed, so add it
      if collection:
        div.append(h)
        div.append(collection)
        if div not in el:
          el.append(div)

      # If the table does not exist then only add this section if the groups are active
      elif action.group in groups:
        div.append(h)
        if div not in el:
          el.append(div)

    return el
