#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import logging
from ..common import exceptions
from ..base import components, LatexRenderer, MarkdownReader
from ..tree import tokens, html, latex
from . import command, core, floats

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    return AlgorithmExtension(**kwargs)

Algorithm = tokens.newToken('Algorithm')
AlgorithmComponent = tokens.newToken('AlgorithmComponent', line=0, level=0, content=None, comment=None)
AlgorithmFloat = tokens.newToken('AlgorithmFloat', floats.Float)

class AlgorithmExtension(command.CommandExtension):
    """
    Adds commands needed to create algorithm pseudo-code
    """

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['prefix'] = ('Algorithm', "The caption prefix (e.g., Alg.).")
        return config

    def __init__(self, *args, **kwargs):
        command.CommandExtension.__init__(self, *args, **kwargs)

        self.reset()

    def extend(self, reader, renderer):
        self.requires(core, command, floats)

        self.addCommand(reader, AlgorithmCommand())
        self.addCommand(reader, FunctionComponent())
        self.addCommand(reader, LoopComponent())
        self.addCommand(reader, StatementComponent())
        self.addCommand(reader, IfThenComponent())

        renderer.add('Algorithm', RenderAlgorithm())
        renderer.add('AlgorithmComponent', RenderAlgorithmComponent())
        renderer.add('AlgorithmFloat', RenderAlgorithmFloat())

        if isinstance(renderer, LatexRenderer):
            renderer.addPackage('algpseudocode')

    def reset(self):
        self._current_line = 0
        self._current_level = 0

    def newLine(self, level_increment=0):
        self._current_line += 1
        self._current_level += level_increment
        if (self._current_level < 0):
            self._current_level = 0

    @property
    def line(self):
        return self._current_line

    @property
    def level(self):
        return self._current_level

class AlgorithmCommand(command.CommandComponent):
    COMMAND = 'algorithm'
    SUBCOMMAND = None

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings.update(floats.caption_settings())
        return settings

    def createToken(self, parent, info, page, settings):
        content = info['block'] if 'block' in info else info['inline']
        self.extension.reset()
        flt = floats.create_float(parent, self.extension, self.reader, page, settings, token_type=AlgorithmFloat)
        algorithm = Algorithm(flt)
        self.reader.tokenize(algorithm, content, page, MarkdownReader.INLINE, line=info.line)
        if flt is parent:
            algorithm.attributes.update(**self.attributes(settings))
        return parent

class AlgorithmComponentBase(command.CommandComponent):

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['comment'] = (None, 'Comment placed to right of algorithm component.')
        return settings

    def createTokenHelper(self, parent, info, page, settings, content='', begin_env=False, end_env=False):
        level = self.extension.level
        if begin_env and end_env:
            self.extension.newLine(0)
            level -= 1
        elif begin_env:
            self.extension.newLine(1)
        elif end_env:
            self.extension.newLine(-1)
            level -= 1
        else:
            self.extension.newLine(0)

        line = self.extension.line

        cnt = None
        if content is not None:
            cnt = tokens.Token(None)
            self.reader.tokenize(cnt, content, page, MarkdownReader.INLINE, line=info.line)

        cmt = None
        comment = settings['comment']
        if comment is not None:
            comment = '&#9657; {}'.format(comment)
            cmt = tokens.Token(None)
            self.reader.tokenize(cmt, comment, page, MarkdownReader.INLINE, line=info.line)

        return AlgorithmComponent(parent, line=line, level=level, content=cnt, comment=cmt)

class FunctionComponent(AlgorithmComponentBase):
    COMMAND = ('procedure', 'function')
    SUBCOMMAND = ('begin', 'end')

    @staticmethod
    def defaultSettings():
        settings = AlgorithmComponentBase.defaultSettings()
        settings['name'] = (None, 'Function name')
        settings['param'] = (None, 'Function parameters')
        return settings

    def createToken(self, parent, info, page, settings):
        is_end =  info['subcommand'] == 'end'
        if is_end:
            content = '+end {}+'.format(info['command'])
        else:
            content = '+{}+'.format(info['command'])
            if settings['name'] is not None:
                content += ' ' + settings['name']
            if settings['param'] is not None:
                content += '({})'.format(settings['param'])
        return AlgorithmComponentBase.createTokenHelper(self, parent, info, page, settings, content, begin_env=not is_end, end_env=is_end)

class LoopComponent(AlgorithmComponentBase):
    COMMAND = ('for', 'while')
    SUBCOMMAND = ('begin', 'end')

    @staticmethod
    def defaultSettings():
        settings = AlgorithmComponentBase.defaultSettings()
        settings['condition'] = ('', 'Loop condition')
        return settings

    def createToken(self, parent, info, page, settings):
        is_end =  info['subcommand'] == 'end'
        if is_end:
            content = '+end {}+'.format(info['command'])
        else:
            content = '+{}+ {} +do+'.format(info['command'], settings['condition'])

        return AlgorithmComponentBase.createTokenHelper(self, parent, info, page, settings, content, begin_env=not is_end, end_env=is_end)

class StatementComponent(AlgorithmComponentBase):
    COMMAND = 'state'
    SUBCOMMAND = None

    @staticmethod
    def defaultSettings():
        settings = AlgorithmComponentBase.defaultSettings()
        settings['text'] = ('', 'Statement text')
        return settings

    def createToken(self, parent, info, page, settings):
        return AlgorithmComponentBase.createTokenHelper(self, parent, info, page, settings, content=settings['text'])

class IfThenComponent(AlgorithmComponentBase):
    COMMAND = 'ifthen'
    SUBCOMMAND = ('if', 'elif', 'else', 'end')

    @staticmethod
    def defaultSettings():
        settings = AlgorithmComponentBase.defaultSettings()
        settings['condition'] = ('', 'If and if else condition')
        return settings

    def createToken(self, parent, info, page, settings):
        content = ''
        env = [True, True]
        if info['subcommand'] == 'if':
            content = '+if+ {} +then+'.format(settings['condition'])
            env[1] = False
        elif info['subcommand'] == 'elif':
            content = '+else if+ {} +then+'.format(settings['condition'])
        elif info['subcommand'] == 'else':
            content = '+else+'
        elif info['subcommand'] == 'end':
            content = '+end if+'
            env[0] = False

        return AlgorithmComponentBase.createTokenHelper(self, parent, info, page, settings, content, begin_env=env[0], end_env=env[1])

class RenderAlgorithm(components.RenderComponent):
    def createHTML(self, parent, token, page):
        div = html.Tag(parent, 'div')
        div.addClass('moose-algorithm')
        return div

    def createMaterialize(self, parent, token, page):
        return self.createHTML(parent, token, page)

    def createLatex(self, parent, token, page):
        pass

class RenderAlgorithmFloat(floats.RenderFloat):

    def createLatex(self, parent, token, page):
        pass

class RenderAlgorithmComponent(components.RenderComponent):

    def createMaterialize(self, parent, token, page):
        return self.createHTML(parent, token, page)

    def createHTML(self, parent, token, page):
        div = html.Tag(parent, 'div')
        div.addClass('moose-algorithm-line')


        line = '{}: '.format(token['line'])
        span1 = html.Tag(div, 'span')
        html.Tag(span1, 'span', string=line, style='font-size:80%')
        html.String(span1, content='&nbsp;'*(5*token['level'] + 1))

        if token['content'] is not None:
            self.renderer.render(span1, token['content'], page)

        span2 = html.Tag(div, 'span')
        if token['comment'] is not None:
            self.renderer.render(span2, token['comment'], page)
        return parent

    def createLatex(self, parent, token, page):
        pass
