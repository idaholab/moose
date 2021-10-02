#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
from ..base import components, LatexRenderer, HTMLRenderer, MarkdownReader
from ..tree import tokens, html, latex
from . import command, materialicon

def make_extension(**kwargs):
    return AlertExtension(**kwargs)

AlertToken = tokens.newToken('AlertToken', brand='')
AlertTitle = tokens.newToken('AlertTitle', brand='', prefix=True, center=False, icon=True, icon_name=None)
AlertContent = tokens.newToken('AlertContent')

# LaTeX alert environment that uses tcolorbox package
ALERT_LATEX = """\\setlength\\intextsep{0pt}
\\NewDocumentEnvironment{alert}{O{#2}moO{white}}{%
  \\ifthenelse{\\isempty{#1}}{%
      \\IfValueT{#3}{\\tcbset{title=#3}}
    }{%
      \\tcbset{title=\\MakeUppercase{#1}\\IfValueT{#3}{: #3}}
    }
  \\begin{tcolorbox}[arc=0mm,fonttitle=\\bfseries,colback=alert-#2!5,colframe=alert-#2,coltitle=#4]
}{%
  \\end{tcolorbox}
}
"""

class AlertExtension(command.CommandExtension):
    """
    Adds alert boxes (note, tip, error, warning, and construction) to display important information.
    """

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['use-title-prefix'] = (True, "Enable/disable including the brand (e.g., ERROR) as " \
                                            "prefix for the alert title.")
        return config

    def extend(self, reader, renderer):
        self.requires(command)
        self.addCommand(reader, AlertCommand())
        renderer.add('AlertToken', RenderAlertToken())
        renderer.add('AlertTitle', RenderAlertTitle())
        renderer.add('AlertContent', RenderAlertContent())

        if isinstance(renderer, LatexRenderer):
            renderer.addPackage('xcolor')
            renderer.addPackage('xparse')
            renderer.addPackage('xifthen')
            renderer.addPackage('tcolorbox')
            renderer.addPackage('wrapfig')
            renderer.addPackage('graphicx')

            renderer.addPreamble('\\definecolor{alert-error}{RGB}{153,0,0}')
            renderer.addPreamble('\\definecolor{alert-note}{RGB}{0,88,151}')
            renderer.addPreamble('\\definecolor{alert-warning}{RGB}{220,200,100}')
            renderer.addPreamble('\\definecolor{alert-info}{RGB}{0,128,21}')
            renderer.addPreamble('\\definecolor{alert-construction}{RGB}{255,114,33}')
            renderer.addPreamble(ALERT_LATEX)

        if isinstance(renderer, HTMLRenderer):
            renderer.addCSS('alert_moose', "css/alert_moose.css")

class AlertCommand(command.CommandComponent):
    COMMAND = 'alert'
    SUBCOMMAND = ('error', 'warning', 'note', 'tip', 'construction')

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['title'] = (None, "The optional alert title.")
        settings['center-title'] = (False, "Center the title.")
        settings['prefix'] = (None, "Enable/disable the title being prefixed with the alert brand.")
        settings['icon'] = (True, "Enable/disable the title icon.")
        settings['icon-name'] = (None, "Set the icon name, see material icon for available options.")
        return settings

    def createToken(self, parent, info, page, settings):
        title = settings.pop('title', None)
        brand = info['subcommand']
        icon_name = settings['icon-name']
        if icon_name is None:
            if brand == 'note':
                icon_name = 'comment'
            elif brand == 'construction':
                icon_name = 'build'
            elif brand == 'tip':
                icon_name = 'school'
            elif brand == 'error':
                icon_name = 'report'
            else:
                icon_name = brand

        if settings['prefix'] is not None:
            prefix = settings['prefix']
        else:
            prefix = self.extension.get('use-title-prefix', True)

        alert_token = AlertToken(parent, brand=brand)
        title_token = AlertTitle(alert_token, prefix=prefix, brand=brand,
                                 icon=settings['icon'],
                                 icon_name=icon_name,
                                 center=settings['center-title'])

        if title:
            self.reader.tokenize(title_token, title, page, MarkdownReader.INLINE)

        return AlertContent(alert_token, brand=brand)

class RenderAlertToken(components.RenderComponent):

    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'div', class_='moose-alert moose-alert-{}'.format(token['brand']))

    def createMaterialize(self, parent, token, page):
        return html.Tag(parent, 'div',
                        class_='card moose-alert moose-alert-{}'.format(token['brand']))

    def createLatex(self, parent, token, page):

        # Argument list (see ALERT above)
        args = []
        if token(0)['prefix']:
            args.append(latex.Bracket(string=token['brand']))
        else:
            args.append(latex.Bracket())

        args.append(latex.Brace(string=token['brand']))

        if token(0).children:
            title = latex.Bracket()
            self.renderer.render(title, token(0), page)
            args.append(title)

        env = latex.Environment(parent, 'alert', args=args)

        token(0).parent = None
        return env

class RenderAlertContent(components.RenderComponent):

    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'div', class_='moose-alert-content')

    def createMaterialize(self, parent, token, page):

        card_content = html.Tag(parent, 'div', class_='card-content')
        content = html.Tag(card_content, 'div', class_='moose-alert-content')
        return content

    def createLatex(self, parent, token, page):
        return parent

class RenderAlertTitle(components.RenderComponent):

    def createHTML(self, parent, token, page):
        title = html.Tag(parent, 'div', class_='moose-alert-title')

        if token.get('icon'):
            i = html.Tag(title, 'i', token, string=token['icon_name'])
            i.addClass('material-icons')
            i.addClass('moose-inline-icon')

        if token.get('prefix'):
            brand = token['brand']
            prefix = html.Tag(title, 'span', string=brand, class_='moose-alert-title-brand')
            if token.children:
                html.String(prefix, content=': ')

        return title

    def createMaterialize(self, parent, token, page):

        title = html.Tag(parent, 'div', class_='card-title moose-alert-title')
        if token.get('icon'):
            i = html.Tag(title, 'i', token, string=token['icon_name'])
            i.addClass('material-icons')
            i.addClass('moose-inline-icon')

        if token.get('prefix'):
            brand = token['brand']
            prefix = html.Tag(title, 'span', string=brand, class_='moose-alert-title-brand')
            if token.children:
                html.String(prefix, content=':')

        if token.get('center'):
            title.addClass('center')

        return title

    def createLatex(self, parent, token, page):
        return parent
