#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import re
import logging
from MooseDocs.base import renderers, components
from MooseDocs.tree import html, tokens
from MooseDocs.extensions import core, command

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    """Create an instance of the Extension object."""
    return RevealExtension(**kwargs)

Section = tokens.newToken('Section')
SubSection = tokens.newToken('SubSection')
Notes = tokens.newToken('Notes')

class RevealExtension(command.CommandExtension):
    """
    Presentation extension using Reveal.js
    """
    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        return config

    def initMetaData(self, page, meta):
        meta.initData('pdf-active', True)

    def extend(self, reader, renderer):
        """
        Add the necessary components for reading and rendering LaTeX.
        """
        self.requires(core)

        if not isinstance(renderer, renderers.RevealRenderer):
            self.setActive(False)

        else:
            reader.addBlock(SectionBlock(), 0)
            renderer.add('Section', RenderSection())
            renderer.add('SubSection', RenderSection())

        self.addCommand(reader, RevealNotes())
        renderer.add('Notes', RenderNotes())

    def postTokenize(self, ast, page, meta, reader):
        ast.insert(0, Section(None))

        section = None
        for child in ast.children:
            if child.name == 'Section':
                section = child
            else:
                child.parent = section

        for child in ast.children:
            has_sub_section = any([c.name == 'SubSection' for c in child.children])
            if has_sub_section:
                section = None
                child.insert(0, SubSection(None))
                for subchild in child.children:
                    if subchild.name == 'SubSection':
                        section = subchild
                    else:
                        subchild.parent = section

class RevealNotes(command.CommandComponent):
    COMMAND = 'notes'
    SUBCOMMAND = None

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        return settings

    def createToken(self, parent, info, page):
        return Notes(parent)

class SectionBlock(components.TokenComponent):
    RE = re.compile(r'\s*(?P<SlideBreak>^!-{2,3}$)',
                    flags=re.UNICODE|re.MULTILINE|re.DOTALL)

    @staticmethod
    def defaultSettings():
        settings = components.TokenComponent.defaultSettings()
        return settings

    def createToken(self, parent, info, page):
        n = len(info['SlideBreak'])
        if n == 2:
            SubSection(parent)
        else:
            Section(parent)
        return parent

class RenderSection(components.RenderComponent):
    def createReveal(self, parent, token, page):
        return html.Tag(parent, 'section')


class RenderNotes(components.RenderComponent):
    def createReveal(self, parent, token, page):
        return html.Tag(parent, 'aside', class_='notes')
