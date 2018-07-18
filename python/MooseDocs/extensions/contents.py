#pylint: disable=missing-docstring

import anytree
from MooseDocs.base import components
from MooseDocs.common import exceptions
from MooseDocs.tree import page, tokens, html
from MooseDocs.extensions import command

def make_extension(**kwargs):
    return ContentsExtension(**kwargs)

class ContentsToken(tokens.Token):
    """
    Token for source code tree for the supplied page node object.
    """
    PROPERTIES = [tokens.Property("node")]

class ContentsExtension(command.CommandExtension):
    """
    Allows for the creation of markdown contents lists.
    """

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        return config

    def extend(self, reader, renderer):
        self.requires(command)
        self.addCommand(ContentsCommand())
        renderer.add(ContentsToken, RenderContents())

class ContentsCommand(command.CommandComponent):
    COMMAND = 'contents'
    SUBCOMMAND = None

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['location'] = (None, "The markdown content directory to build contents.")
        return settings

    def createToken(self, info, parent):

        location = self.settings['location']
        if location is None:
            node = self.translator.root
        else:
            func = lambda n: n.name == location and isinstance(n, page.DirectoryNode)
            node = anytree.search.find(self.translator.root, filter_=func)
            if not node:
                raise exceptions.TokenizeException("Unable to locate the directory '{}'.", location)

        ContentsToken(parent, node=node)
        return parent


class RenderContents(components.RenderComponent):
    def createHTML(self, token, parent):
        self._dump(parent, token.node)

    def _dump(self, parent, node, level=2):

        ul = html.Tag(parent, 'ul')
        items = [] # storage for non-directories to allow for directories to list first
        for child in node.children:
            li = html.Tag(None, 'li')

            # ignores source/index.md
            if child is self.translator.current:
                continue

            # Directory
            elif isinstance(child, page.DirectoryNode):
                text = html.Tag(None, 'span',
                                string=unicode(child.name),
                                class_='moose-source-directory')

            # File
            else:
                loc = child.relativeDestination(self.translator.current)
                text = html.Tag(None, 'a',
                                string=unicode(child.name.replace('.md', '')),
                                href=loc,
                                class_='moose-source-file')

            # Enable scrollspy for top-level, see renderers.py for how this works
            if level == 2:
                li['data-section-level'] = level
                li['data-section-text'] = child.name
                li['data-section-text'] = text.text()
                li['id'] = text.text().lower().replace(' ', '-')
                text['class'] = 'moose-source-directory'

            # Create nested, collapsible list of children
            if child.children:
                details = html.Tag(ul, 'details', open='open')
                summary = html.Tag(details, 'summary')
                html.Tag(summary, 'span', class_='moose-section-icon')
                text.parent = summary

                li.parent = details
                self._dump(li, child, level + 1)

            else:
                text.parent = li
                items.append(li)

        # Add file items to the list
        for li in items:
            li.parent = ul
