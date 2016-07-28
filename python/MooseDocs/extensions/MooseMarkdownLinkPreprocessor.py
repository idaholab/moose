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

    def __init__(self, md, database_dir, *args, **kwargs):
        super(MooseMarkdownLinkPreprocessor, self).__init__(md, *args, **kwargs)
        self._database_dir = database_dir
        self._database = None

    def run(self, lines):
        """
        The raw markdown lines are processed to find and create links between pages.
        """

        # Get current filename (this is must be injected as a comment on the first line of the input)
        match = re.search(r'<!--\s*(.*?)\s*-->', lines[0])
        if match:
            local = os.path.dirname(match.group(1))
        else:
            log.warning("Unable to read markdown filename from injected comment.")
            return lines

        # Loop through each line and create autolinks
        for i in range(len(lines)):
            lines[i] = re.sub(r'(?<!`)\[auto::(.*?\.md)\]', lambda m: self.bracketSub(local, m), lines[i])
            lines[i] = re.sub(r'(?<!`)\[auto::(.*?)\]\((.*?\.md)\)', lambda m: self.linkSub(local, m), lines[i])
        return lines

    def buildDatabase(self):
        self._database = MooseDocs.database.Database('.md', self._database_dir, MooseDocs.database.items.MarkdownIncludeItem)

    def bracketSub(self, local, match):
        """
        Substitution of bracket links: [Diffusion.md]

        Args:
            local[str]: The path of the current markdown file.
            match[re.Match]: The python re.Match object.
        """

        # Locate database items given the key
        name = match.group(1)

        # Check if the name exists, if it does then auto-link is not needed
        if os.path.exists(os.path.join(os.path.dirname(local), name)):
            return match.group(0)

        # Build the database if needed
        if not self._database:
            self.buildDatabase()

        # Locate the markdown files matching the supplied name
        items = self._database.findall(name)
        self.checkMultipleItems(items, name)

        # Make link subsitution
        if items:
            fname = items[0][0].filename()
            rel = os.path.relpath(fname, local)
            syntax, _ = os.path.splitext(name)
            if not syntax.endswith('Overview'):
                syntax = os.path.basename(syntax)
            return '[{}]({})'.format(syntax, rel)

        # Do nothing
        else:
            return match.group(0)

    def linkSub(self, local, match):
        """
        Substitution of links: [Test](Diffusion.md)

        Args:
            local[str]: The path of the current markdown file.
            match[re.Match]: The python re.Match object.
        """

        # Locate database items given the key
        name = match.group(2)

        # Check if the name exists, if it does then auto-link is not needed
        if os.path.exists(os.path.join(os.path.dirname(local), name)):
            return match.group(0)

        # Build the database if needed
        if not self._database:
            self.buildDatabase()

        # Locate the markdown files matching the supplied name
        items = self._database.findall(name)
        self.checkMultipleItems(items, name)

        # Make link subsitution
        if items:
            fname = items[0][0].filename()
            rel = os.path.relpath(fname, local)
            return '[{}]({})'.format(match.group(1), rel)

        # Do nothing
        else:
            return match.group(0)


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
            raise Exception('done')
