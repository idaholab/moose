import logging
log = logging.getLogger(__name__)

from markdown.util import etree
from MooseSyntaxBase import MooseSyntaxBase

class MooseObjectSyntax(MooseSyntaxBase):
    """
    Extracts the description from a MooseObject parameters.

    Markdown Syntax:
    !<Keyword> <YAML Syntax> key=value, key1=value1, etc...

    Keywords Available:
      !inputfiles - Returns a set of lists containing links to the input files that use the syntax
      !childobjects - Returns a set of lists containing links to objects that inherit from this class
    """

    RE = r'^!(inputfiles|childobjects)\s+(.*?)(?:$|\s+)(.*)'

    def __init__(self, database=None, repo=None, **kwargs):
        super(MooseObjectSyntax, self).__init__(self.RE, **kwargs)

        # Input arguments
        self._input_files = database.inputs
        self._child_objects = database.children
        self._repo = repo

    def handleMatch(self, match):
        """
        Create a <p> tag containing the supplied description from the YAML dump.
        """

        # Extract match options and settings
        action = match.group(2)
        syntax = match.group(3)

        # Extract Settings
        settings = self.getSettings(match.group(4))

        # Locate description
        info = self.getObject(syntax)
        if not info:
            el = self.createErrorElement('Failed to locate MooseObject with syntax in command: !{} {}'.format(action, syntax), error=False)
        elif action == 'inputfiles':
            el = self.inputfilesElement(info, settings)
        elif action == 'childobjects':
            el = self.childobjectsElement(info, settings)
        return el

    def inputfilesElement(self, info, settings):
        """
        Return the links to input files and child objects.

        Args:
          node[dict]: YAML data node.
          styles[dict]: Styles from markdown.
        """
        # Print the item information
        el = self.applyElementSettings(etree.Element('div'), settings)
        el.set('id', '#input-files')
        el.set('class', 'section scrollspy')
        self._listhelper(info, 'Input Files', el, self._input_files)
        return el

    def childobjectsElement(self, info, settings):
        """
        Return the links to input files and child objects.

        Args:
          node[dict]: YAML data node.
          styles[dict]: Styles from markdown.
        """
        # Print the item information
        el = self.applyElementSettings(etree.Element('div'), settings)
        el.set('id', '#child-objects')
        el.set('class', 'section scrollspy')
        self._listhelper(info, 'Child Objects', el, self._child_objects)
        return el

    def _listhelper(self, info, title, parent, items):
        """
        Helper method for dumping link lists.

        Args:
          info: MooseObjectInfo object.
          title[str]: The level two header to apply to lists.
          parent[etree.Element]: The parent element the headers and lists are to be applied
          items[dict]: Dictionary of databases containing link information
        """
        has_items = False
        for k, db in items.iteritems():
            if info.name in db:
                has_items = True
                h3 = etree.SubElement(parent, 'h3')
                h3.text = k
                ul = etree.SubElement(parent, 'ul')
                ul.set('style', "max-height:350px;overflow-y:Scroll")
                for j in db[info.name]:
                    ul.append(j.html())

        if has_items:
            h2 = etree.Element('h2')
            h2.text = title
            parent.insert(0, h2)
