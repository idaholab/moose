#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import pyhit
import moosetree
import difflib

from .. import common
from ..common import exceptions
from ..base import LatexRenderer
from ..tree import html, tokens, latex
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
    DEFAULT_MAX_HEIGHT = 350

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings.update(floats.caption_settings())
        settings['max-height'] = (f'{LocalListingCommand.DEFAULT_MAX_HEIGHT}px', "The default height for listing content.")
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
    DEFAULT_BEFORE_LINK_PREFIX = '-'
    DEFAULT_AFTER_LINK_PREFIX = '+'

    @staticmethod
    def defaultSettings():
        settings = LocalListingCommand.defaultSettings()
        settings['diff'] = (None, 'Path to a file to diff against')
        settings['before_link_prefix'] = (FileListingCommand.DEFAULT_BEFORE_LINK_PREFIX,
                                          'Prefix for the modal link to the diffed "before" file')
        settings['after_link_prefix'] = (FileListingCommand.DEFAULT_AFTER_LINK_PREFIX,
                                         'Prefix for the modal link to the diffed "after" file')
        settings['link'] = (True, "Show the complete file via a link; overridden by SourceExtension")
        settings.update(common.extractContentSettings())
        return settings

    def getContent(self, filename, settings):
        """
        Base method for extracting content from a file.

        Allows specialized commands to filter content.
        """
        return common.read(filename)

    def createToken(self, parent, info, page, settings):
        """
        Build the tokens needed for displaying code listing.
        """
        def get_content(filename):
            content = self.getContent(filename, settings)
            return common.extractContent(content, settings)[0]

        filename = common.check_filenames(info['subcommand'])
        content = get_content(filename)

        # Create code token
        lang = settings.get('language')
        lang = lang if lang else common.get_language(filename)
        # Language for the ModelSourceLink; independent of the
        # Code language when we're doing a diff
        link_lang = lang

        # Diff against a file
        if settings['diff']:
            before_filename = common.check_filenames(settings['diff'])
            before_content = get_content(before_filename)
            content = self.codeDiff(content, before_content)
            lang = f'diff-{lang} diff-highlight'

        flt = floats.create_float(parent, self.extension, self.reader, page, settings,
                                  token_type=Listing)

        code = core.Code(flt, style="max-height:{};".format(settings['max-height']),
                         content=content, language=lang)

        if flt is parent:
            code.attributes.update(**self.attributes(settings))
        else:
            code.name = 'ListingCode' #TODO: Find a better way

        if settings['link']:
            if settings['diff']:
                modal.ModalSourceLink(flt, src=before_filename, language=link_lang,
                                      link_prefix=settings['before_link_prefix'])
                html.Tag(flt, 'link_break', string='<br>')
            link_prefix = (settings['after_link_prefix']) if settings['diff'] else None
            modal.ModalSourceLink(flt, src=filename, language=link_lang,
                                  link_prefix=link_prefix)

        return parent

    @staticmethod
    def codeDiff(before: str, after: str) -> str:
        """
        Helper for producing a unified diff of content.

        All of the context is kept with the diff, that is,
        the full file is shown continuously instead of being
        split up if diffs are minor

        Args:
            before: The before content
            after: The after content
        Returns:
            str: The diff
        """
        # Content needs to be split into a list for difflib
        before_split = before.splitlines(True)
        after_split = after.splitlines(True)

        # difflib won't produce anything if they're the same, so
        # to mimc a diff we need to add a new empty space to each line
        if before_split == after_split:
            return ' ' + '\n '.join(after_split)

        # Produce the diff here; n sets how much context to add (add all of it)
        diff = difflib.unified_diff(after_split, before_split,
                                    n=(len(before_split) + len(after_split)))
        # Re-join the split diff but split the header (via the [3:])
        return ' ' + ''.join(list(diff)[3:])

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

    def getContent(self, filename, settings):
        """
        Specialized method for extracting content from a file.

        Allows for filtering of input files by capturing
        blocks or removing blocks.
        """
        if any([settings['block'], settings['remove']]):
            hit = self.extractInputBlocks(filename, settings['block'] or '')
            return self.removeInputBlocks(hit, settings['remove'] or '')
        return super().getContent(filename, settings)

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
            # This render doesn't include the parents (if any), but...
            # we want the parents. There's zero editing capability in pyhit,
            # so we're going to lazily replace the name with the full path :/
            render = str(node.render())
            if node.parent != hit:
                render = render.replace(f'[{node.name}]', f'[{node.fullpath.strip("/")}]', 1)
            out.append(render)
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
