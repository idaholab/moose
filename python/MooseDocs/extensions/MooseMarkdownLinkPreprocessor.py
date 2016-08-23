import os
import re
from markdown.preprocessors import Preprocessor
import logging
log = logging.getLogger(__name__)
import MooseDocs

class MooseMarkdownLinkPreprocessor(Preprocessor):
    """
    A preprocessor for creating automatic linking between markdown files.
    """

    def __init__(self, database_dir, *args, **kwargs):
        super(MooseMarkdownLinkPreprocessor, self).__init__(*args, **kwargs)
        self._database_dir = database_dir
        self._database = None

    def run(self, lines):
        """
        The raw markdown lines are processed to find and create links between pages.
        """

        # Loop through each line and create autolinks
        for i in range(len(lines)):
            lines[i] = re.sub(r'(?<!`)\[auto::(.*?)\]', lambda m: self.bracketSub(m), lines[i])
            lines[i] = re.sub(r'(?<!`)\[(.*?)\]\(auto::(.*?)\)', lambda m: self.linkSub(m), lines[i])
        return lines

    def buildDatabase(self):
        self._database = MooseDocs.database.Database('.md', self._database_dir, MooseDocs.database.items.MarkdownIncludeItem)

    def bracketSub(self, match):
        """
        Substitution of bracket links: [Diffusion.md]

        Args:
            local[str]: The path of the current markdown file.
            match[re.Match]: The python re.Match object.
        """

        # Locate database items given the key
        name = match.group(1)

        # Build the database if needed
        if not self._database:
            self.buildDatabase()

        # Locate the markdown files matching the supplied name
        items = self._database.findall(name)
        self.checkMultipleItems(items, name)

        # Make link substitution
        if len(items) > 0:
            fname = items[0][0].filename()
            rel, _ = os.path.splitext(os.path.relpath(fname, os.getcwd()))
            syntax, _ = os.path.splitext(name)
            return '[{}](/{})'.format(syntax, rel)

        # Do nothing
        else:
            return self.admonition(match.group(0))

    def linkSub(self, match):
        """
        Substitution of links: [Test](Diffusion.md)

        Args:
            local[str]: The path of the current markdown file.
            match[re.Match]: The python re.Match object.
        """

        # Locate database items given the key
        name = match.group(2)

        # Build the database if needed
        if not self._database:
            self.buildDatabase()

        # Locate the markdown files matching the supplied name
        items = self._database.findall(name)
        self.checkMultipleItems(items, name)

        # Make link subsitution
        if items:
            fname = items[0][0].filename()
            rel = os.path.relpath(fname, os.getcwd())
            return '[{}](/{})'.format(match.group(1), rel)

        # Do nothing
        else:
            return self.admonition(match.group(0))

    @staticmethod
    def checkMultipleItems(items, name):
        """
        Helper for warning if multiple items are found.
        """
        if len(items) > 1:
            msg = "Found multiple items listed with the name '{}':".format(name)
            for item in items:
                msg += '\n    {}'.format(item[0].filename())
            log.warning(msg)

    @staticmethod
    def admonition(link):
        log.error('A page failed to locate the auto-link: {}'.format(link))
        return '\n\n!!! danger "Auto Link Failed!"\n    A MOOSE markdown automatic link failed: `{}`\n\n'.format(link)
