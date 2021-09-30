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
import moosetree
from ..base import renderers, components
from ..tree import html, tokens
from . import core, command

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
        config['translate'] = (['index.md'], "The files that should be translated, this allows " \
                               "files that are included to only be translated once.")
        config['background_image'] = (None, "Background image for all slides.")
        return config

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

    def postRead(self, page, content):
        """Deactivate all pages not listed for translation.

        This allows for include pages to only be translated when included which improves performance
        when building a single file such as the case for presentations.
        """
        if page.local not in self.get('translate'):
            page['active'] = False

    def postTokenize(self, page, ast):
        ast.insert(0, Section(None))

        section = None
        for child in ast.children:
            if child.name == 'Section':
                section = child
                child['data-background'] = 'https://ukmadcat.com/wp-content/uploads/2019/04/sleepy-cat.jpg'
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

    def preWrite(self, page, result):
        """Update internal links to use slide numbers."""

        # The Reveal.js platform does not honor traditional anchors in links, so the following
        # populates a lookup dictionary for each "id" to the associated slide number(s). Then, all
        # links to these anchors are update to use the slide number.
        lookup = dict()
        thesection = 0
        thesubsection = 0
        for section in result:
            thesubsection = 0
            for subsection in section:
                for key in self._getNodeIDs(subsection):
                    lookup[key] = (thesection, thesubsection)
                thesubsection += 1
            thesection += 1

        for a in moosetree.iterate(result, lambda n: n.name == 'a'):
            href = a.get('href', '')
            if href.startswith('#'):
                key = lookup[href[1:]]
                a['href'] = '#/{}/{}'.format(*key)

        img = self.get('background_image')
        if (img is not None) and (not img.startswith('http')):
            node = self.translator.findPage(img)
            img = str(node.relativeSource(page))

        if img is not None:
            for sec in moosetree.iterate(result, lambda n: n.name == 'section'):
                sec['data-background'] = img

    @staticmethod
    def _getNodeIDs(root):
        """Return all 'ids' in a node tree."""
        keys = []
        id_ = root.get('id', None)
        if id_:
            keys.append(id_)

        for node in moosetree.iterate(root, method=moosetree.IterMethod.PRE_ORDER):
            id_ = node.get('id', None)
            if id_:
                keys.append(id_)

        return keys

class RevealNotes(command.CommandComponent):
    COMMAND = 'notes'
    SUBCOMMAND = None

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        return settings

    def createToken(self, parent, info, page, settings):
        return Notes(parent)

class SectionBlock(components.ReaderComponent):
    RE = re.compile(r'\s*(?P<SlideBreak>^!-{2,3}$)',
                    flags=re.UNICODE|re.MULTILINE|re.DOTALL)

    @staticmethod
    def defaultSettings():
        settings = components.ReaderComponent.defaultSettings()
        return settings

    def createToken(self, parent, info, page, settings):
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
