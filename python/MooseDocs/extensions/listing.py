#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import pyhit
import moosetree
import mooseutils
import MooseDocs
from .. import common
from ..common import exceptions
from ..base import LatexRenderer
from ..tree import tokens, latex
from . import core, command, floats, modal

Listing = tokens.newToken('Listing', floats.Float)
ListingCode = tokens.newToken('ListingCode', core.Code)
ListingLink = tokens.newToken('ListingLink', core.Link)

def make_extension(**kwargs):
    return ListingExtension(**kwargs)

class ListingExtension(command.CommandExtension):
    """
    Provides !listing command for including source code from files.
    """

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['prefix'] = ('Listing', "The caption prefix (e.g., Fig.).")
        config['modal-link'] = (True, "Insert modal links with complete files.")
        return config

    def extend(self, reader, renderer):
        self.requires(core, command, floats, modal)
        self.addCommand(reader, LocalListingCommand())
        self.addCommand(reader, FileListingCommand())
        self.addCommand(reader, InputListingCommand())

        renderer.add('Listing', RenderListing())
        renderer.add('ListingCode', RenderListingCode())
        renderer.add('ListingLink', RenderListingLink())

        if isinstance(renderer, LatexRenderer):
            renderer.addPackage('listings')
            renderer.addPackage('xcolor')

            renderer.addPreamble("\\lstset{"
                                 "basicstyle=\\ttfamily,"
                                 "columns=fullflexible,"
                                 "frame=single,"
                                 "breaklines=true,"
                                 "showstringspaces=false,"
                                 "showspaces=false,"
                                 "postbreak=\\mbox{\\textcolor{red}{$\\hookrightarrow$}\\space},}")

class LocalListingCommand(command.CommandComponent):
    COMMAND = 'listing'
    SUBCOMMAND = None

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings.update(floats.caption_settings())
        settings['max-height'] = ('350px', "The default height for listing content.")
        settings['language'] = (None, "The language to use for highlighting, if not supplied it " \
                                      "will be inferred from the extension (if possible).")
        return settings

    def createToken(self, parent, info, page, settings):
        flt = floats.create_float(parent, self.extension, self.reader, page, settings,
                                  token_type=Listing)
        content = info['inline'] if 'inline' in info else info['block']
        code = core.Code(flt, style="max-height:{};".format(settings['max-height']),
                         language=settings['language'], content=content)

        if flt is parent:
            code.attributes.update(**self.attributes(settings))
        else:
            code.name = 'ListingCode' #TODO: Find a better way

        return parent

class FileListingCommand(LocalListingCommand):
    COMMAND = 'listing'
    SUBCOMMAND = '*'

    @staticmethod
    def defaultSettings():
        settings = LocalListingCommand.defaultSettings()
        settings['link'] = (True, "Show the complete file via a link; overridden by SourceExtension")
        settings.update(common.extractContentSettings())
        return settings

    def createToken(self, parent, info, page, settings):
        """
        Build the tokens needed for displaying code listing.
        """

        filename = common.check_filenames(info['subcommand'])
        flt = floats.create_float(parent, self.extension, self.reader, page, settings,
                                  token_type=Listing)
        # Create code token
        lang = settings.get('language')
        content = self.extractContent(filename, settings)
        lang = lang if lang else common.get_language(filename)

        code = core.Code(flt, style="max-height:{};".format(settings['max-height']),
                         content=content, language=lang)

        if flt is parent:
            code.attributes.update(**self.attributes(settings))
        else:
            code.name = 'ListingCode' #TODO: Find a better way

        if settings['link']:
            modal.ModalSourceLink(flt, src=filename, language=lang)

        return parent

    def extractContent(self, filename, settings):
        """
        Extract content to display in listing code box.
        """
        content = common.read(filename)
        content, _ = common.extractContent(content, settings)
        return content


class InputListingCommand(FileListingCommand):
    """
    Special listing command for MOOSE hit input files.
    """

    COMMAND = 'listing'
    SUBCOMMAND = ('i', 'hit')

    @staticmethod
    def defaultSettings():
        settings = FileListingCommand.defaultSettings()
        settings['block'] = (None, 'Space separated list of input file block names to include.')
        settings['remove'] = (None, 'Space separated list of input file block and/or parameter ' \
                                    'names to remove. The full path to parameters must be used, ' \
                                    'e.g., `Kernels/diffusion/variable`.')
        return settings

    def extractContent(self, filename, settings):
        """Extract the file contents for display."""
        if any([settings['block'], settings['remove']]):
            hit = self.extractInputBlocks(filename, settings['block'] or '')
            content = self.removeInputBlocks(hit, settings['remove'] or '')
        else:
            content = common.read(filename)

        content, _ = common.extractContent(content, settings)
        return content

    @staticmethod
    def extractInputBlocks(filename, blocks):
        """Read input file block(s)"""
        hit = pyhit.load(filename)
        out = []
        for block in blocks.split():
            node = moosetree.find(hit, lambda n: n.fullpath.endswith(block.strip('/')))
            if node is None:
                msg = "Unable to find block '{}' in {}."
                raise exceptions.MooseDocsException(msg, block, filename)
            out.append(str(node.render()))
        return pyhit.parse('\n'.join(out)) if out else hit

    @staticmethod
    def removeInputBlocks(hit, remove):
        """Remove input file block(s) and/or parameter(s)"""
        for r in remove.split():
            for node in moosetree.iterate(hit):
                block, param = InputListingCommand.removeHelper(node, r)
                if block is not None:
                    if param is None:
                        node.remove()
                    else:
                        node.removeParam(param)
                    break

            if block is None:
                msg = 'Unable to locate block or parameter with name: {}'
                raise exceptions.MooseDocsException(msg, r)

        return str(hit.render())

    @staticmethod
    def removeHelper(node, block):
        if node.fullpath.endswith(block):
            return node, None

        if '/' in block:
            block, param = block.rsplit('/', 1)
            if (node.fullpath.strip('/') == block.strip('/')) and (param in node):
                return node, param

        return None, None

def get_listing_options(token):
    opts = latex.Bracket(None)

    lang = token['language'] or ''
    if lang.lower() == 'cpp':
        lang = 'C++'
    elif lang.lower() == 'text':
        lang = None

    if lang:
        latex.String(opts, content="language={},".format(lang))

    return [opts]

class RenderListing(floats.RenderFloat):

    def createLatex(self, parent, token, page):

        ctoken = token(1)
        opts = get_listing_options(ctoken)

        cap = token(0)
        key = cap['key']
        if key:
            latex.String(opts[0], content="label={},".format(key))

        tok = tokens.Token()
        cap.copyToToken(tok)
        if key:
            latex.String(opts[0], content="caption=")
        else:
            latex.String(opts[0], content="title=")

        if not cap.children:
            latex.String(opts[0], content="\\mbox{}", escape=False)
        else:
            self.translator.renderer.render(latex.Brace(opts[0]), tok, page)

        latex.Environment(parent, 'lstlisting',
                          string=ctoken['content'].strip('\n'),
                          escape=False,
                          after_begin='\n',
                          before_end='\n',
                          args=opts,
                          info=token.info)

        token.children = list()
        return parent

class RenderListingCode(core.RenderCode):

    def createLatex(self, parent, token, page):
        opts = get_listing_options(token)
        latex.Environment(parent, 'lstlisting',
                          string=token['content'].strip('\n'),
                          escape=False,
                          after_begin='\n',
                          before_end='\n',
                          args=opts,
                          info=token.info)
        return parent

class RenderListingLink(core.RenderLink):
    """Removes 'ListingLink' from LaTeX output."""
    def createLatex(self, parent, token, page):
        return None
