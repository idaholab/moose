#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import collections
import logging
import copy
import re
import time
import traceback
import moosetree
import moosesyntax
import mooseutils

import MooseDocs
from .. import common
from ..common import exceptions, report_error
from ..base import components, LatexRenderer, MarkdownReader
from ..tree import html, tokens, latex
from . import command, core, floats, table, autolink, materialicon, modal, alert

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    return AppSyntaxExtension(**kwargs)

ParameterToken = tokens.newToken('ParameterToken', parameter=None, syntax=None, include_heading=True)
InputParametersToken = tokens.newToken('InputParametersToken',
                                       parameters=list(),
                                       level=2,
                                       groups=list(),
                                       hide=list(),
                                       show=list(),
                                       visible=list())

SyntaxList = tokens.newToken('SyntaxList')
SyntaxListItem = tokens.newToken('SyntaxListItem', syntax='', group='', header=False)
SyntaxLink = tokens.newToken('SyntaxLink', core.Link)

LATEX_PARAMETER = """
\\DeclareDocumentEnvironment{InputParameter}{mooo}{%
  \\begin{minipage}{\\textwidth}
  \\textbf{#1} \\newline
  \\smallskip
  \\hfill
  \\begin{minipage}{0.95\\textwidth}
  \\smallskip
}{%
  \\newline
  \\IfValueT{#2}{\\textit{Group}:~#2\\\\}
  \\IfValueT{#3}{\\textit{C++ Type}:~\\texttt{#3}\\\\}
  \\IfValueT{#4}{\\textit{Default}:~#4\\\\}
  \\end{minipage}
  \\end{minipage}
}
"""

LATEX_OBJECT = """
\\DeclareDocumentEnvironment{ObjectDescription}{moo}{%
  \\begin{minipage}{\\textwidth}
  \\textbf{#1} \\newline
  \\smallskip
  \\hfill
  \\begin{minipage}{0.95\\textwidth}
    \\smallskip
}{%
  \\newline
  \\IfValueT{#2}{\\textit{Registered to}~#2\\\\}
  \\IfValueT{#3}{\\textit{Base Type:}~\\texttt{#3}\\\\}
  \\end{minipage}
  \\end{minipage}
}
"""

class AppSyntaxExtension(command.CommandExtension):

    EXTERNAL_MESSAGE = "This page is included from an external application and uses syntax that " \
                       "is not available in the {} application. As such, it is not possible to " \
                       "extract information such as the description or parameters from the " \
                       "associated objects or syntax. These aspects of the page are disabled."

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['executable'] = (None,
                                "The MOOSE application executable to use for generating syntax.")
        config['app_name'] = (None,
                              "The MOOSE application name (e.g., moose_test); if not provided an " \
                              "attempt will be made to determine the name.")
        config['sources'] = ([],
                             "List of source directories to investigate for class information.")
        config['includes'] = ([],
                              "List of include directories to investigate for class information.")
        config['inputs'] = ([],
                            "List of directories to interrogate for input files using an object.")
        config['allow-test-objects'] = (False, "Enable documentation for test objects.");
        config['hide'] = (None, "DEPRECATED")
        config['remove'] = (None, "List or Dictionary of lists of syntax to remove.")
        config['visible'] = (['required', 'optional'],
                             "Parameter groups to show as un-collapsed.")
        config['alias'] = (None, "Dictionary of syntax aliases.")
        config['unregister'] = (None,
                                "A `dict` or `dict` of `dict` including syntax to unregister (key='moose_base', value='parent_syntax')")
        config['markdown'] = (None,
                             "A `dict` or `dict` of `dict` including markdown files to explicitly set a custom, additional markdown reference (key='syntax', value='file.md')")
        config['external_icon'] = ('feedback', "Icon name for the alert title when unavailable syntax is located on an external page.")
        config['external_alert'] = ('warning', "Alert name when unavailable syntax is located on an external page.")
        return config

    def __init__(self, *args, **kwargs):
        command.CommandExtension.__init__(self, *args, **kwargs)

        self._app_type = None
        self._app_syntax = None
        self._app_exe = None
        self._database = None
        self._cache = dict()
        self._object_cache = dict()
        self._syntax_cache = dict()
        self._external_missing_syntax = set() # page.uid

        if self['hide'] is not None:
            LOG.warning("The 'hide' option is no longer being used.")

    def preExecute(self):
        """Populate the application syntax tree."""

        # Don't re-populate
        if self._app_type is not None:
            return

        if self.active and self.get('executable') is None:
            msg = "No executable defined, the 'appsyntax' extension is being disabled."
            LOG.error(msg)
            self.setActive(False)

        if self.active:
            self.__initApplicationSyntax()
            self.__initClassDatabase()

    def __initApplicationSyntax(self):
        """Initialize the application syntax."""

        start = time.time()
        LOG.info("Reading MOOSE application syntax...")
        exe = mooseutils.eval_path(self['executable'])
        exe = mooseutils.find_moose_executable(exe, name=self['app_name'], show_error=False)
        self._app_exe = exe

        if exe is None:
            LOG.error("Failed to locate a valid executable in %s.", self['executable'])
        else:
            try:
                self._app_syntax = moosesyntax.get_moose_syntax_tree(exe,
                                                                     remove=self['remove'],
                                                                     alias=self['alias'],
                                                                     unregister=self['unregister'],
                                                                     markdown=self['markdown'])

                out = mooseutils.runExe(exe, ['--type'])
                match = re.search(r'^MooseApp Type:\s+(?P<type>.*?)$', out, flags=re.MULTILINE)
                if match:
                    self._app_type = match.group("type")
                else:
                    msg = "Failed to determine application type by running the following:\n"
                    msg += "    {} --type".format(exe)
                    LOG.error(msg)

            except Exception as e:
                msg = "Failed to load application executable from '{}' with the following error; " \
                      "application syntax is being disabled.\n".format(exe)
                msg += '\n{}\n'.format(mooseutils.colorText(traceback.format_exc(), 'GREY'))
                msg += "This typically indicates that the application is not producing JSON output " \
                       "correctly, try running the following:\n" \
                       "    {} --json --allow-test-objects\n".format(exe)
                self.setActive(False)
                LOG.error(msg)

        # Enable test objects by removing the test flag (i.e., don't consider them test objects)
        if self['allow-test-objects'] and (self._app_syntax is not None):
            for node in moosetree.iterate(self._app_syntax):
                node.test = False

        LOG.info("MOOSE application syntax complete [%s sec.]", time.time() - start)

    def __initClassDatabase(self):
        """Initialize the class database for faster searching."""

        # Do nothing if the syntax failed to build
        if self._app_syntax is None:
            return

        start = time.time()
        LOG.info("Building MOOSE class database...")
        self._database = common.build_class_database(
            self['sources'], self['includes'], self['inputs'])

        # Cache the syntax entries, search the tree is very slow
        self._cache = dict()
        self._object_cache = dict()
        self._syntax_cache = dict()
        for node in moosetree.iterate(self._app_syntax):
            if not (node.removed or node.test):
                self._cache[node.fullpath()] = node
                if node.alias:
                    self._cache[node.alias] = node
                if isinstance(node, moosesyntax.SyntaxNode):
                    self._syntax_cache[node.fullpath()] = node
                    if node.alias:
                        self._syntax_cache[node.alias] = node
                else:
                    self._object_cache[node.fullpath()] = node
                    if node.alias:
                        self._object_cache[node.alias] = node

        LOG.info("MOOSE class database complete [%s sec]", time.time() - start)

    @property
    def syntax(self):
        return self._app_syntax

    @property
    def database(self):
        return self._database

    @property
    def apptype(self):
        return self._app_type

    @property
    def executable(self):
        return self._app_exe

    def find(self, name, page, node_type=None):

        if name.endswith('<RESIDUAL>'):
            msg = "The use of <RESIDUAL> is no longer needed in the syntax name '%s', it " \
                  "should be removed."
            LOG.warning(msg, name)
            name = name[0:-10]

        node = None
        if node_type == moosesyntax.ObjectNodeBase:
            node = self._object_cache.get(name, None)
        elif node_type == moosesyntax.SyntaxNode:
            node = self._syntax_cache.get(name, None)
        else:
            node = self._cache.get(name, None)

        if node is None:
            if page.external:
                self._external_missing_syntax.add(page.uid)
            else:
                msg = "'{}' syntax was not recognized."
                raise exceptions.MooseDocsException(msg, name)

        return node

    def postTokenize(self, page, ast):
        if page.uid in self._external_missing_syntax:
            brand = self.get('external_alert')
            tok = alert.AlertToken(None, brand=brand)
            alert.AlertTitle(tok, brand=brand, icon_name=self.get('external_icon'),
                             string="Disabled Object Syntax")
            alert.AlertContent(tok, string=self.EXTERNAL_MESSAGE.format(self.apptype))
            ast.insert(0, tok)

    def extend(self, reader, renderer):
        self.requires(core, floats, table, autolink, materialicon, modal, alert)

        self.addCommand(reader, SyntaxDescriptionCommand())
        self.addCommand(reader, SyntaxParametersCommand())
        self.addCommand(reader, SyntaxParameterCommand())
        self.addCommand(reader, SyntaxChildrenCommand())
        self.addCommand(reader, SyntaxInputsCommand())
        self.addCommand(reader, SyntaxListCommand())
        self.addCommand(reader, SyntaxCompleteCommand())

        renderer.add('InputParametersToken', RenderInputParametersToken())
        renderer.add('ParameterToken', RenderParameterToken())
        renderer.add('SyntaxList', RenderSyntaxList())
        renderer.add('SyntaxListItem', RenderSyntaxListItem())
        renderer.add('SyntaxLink', RenderSyntaxLink())

        if isinstance(renderer, LatexRenderer):
            renderer.addPreamble(LATEX_PARAMETER)
            renderer.addPreamble(LATEX_OBJECT)
            renderer.addPackage('xcolor')

class SyntaxCommandBase(command.CommandComponent):
    NODE_TYPE = None
    COMMAND = 'syntax'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['syntax'] = (None, "The name of the syntax to extract. If the name of the syntax "\
                                    "is the first item in the settings the 'syntax=' may be " \
                                    "omitted, e.g., `!syntax parameters /Kernels/Diffusion`.")
        return settings

    def createToken(self, parent, info, page, settings):
        if settings['syntax'] is None:
            args = info['settings'].split()
            if args and ('=' not in args[0]):
                settings['syntax'] = args[0]

        if settings['syntax']:
            obj = self.extension.find(settings['syntax'], page, self.NODE_TYPE)
            if obj is None:
                return self.createDisabledToken(parent, info, page, settings)
        else:
            obj = self.extension.syntax

        return self.createTokenFromSyntax(parent, info, page, obj, settings)

    def createDisabledToken(self, parent, info, page, settings):
        tag = 'span' if MarkdownReader.INLINE in info else 'p'
        tok = tokens.DisabledToken(parent, tag=tag, string=settings['syntax'])
        return parent

    def createTokenFromSyntax(self, parent, info, page, obj, settings):
        pass

class SyntaxCommandHeadingBase(SyntaxCommandBase):
    @staticmethod
    def defaultSettings():
        settings = SyntaxCommandBase.defaultSettings()
        settings['heading'] = ('Input Parameters',
                               "The heading title for the input parameters table, use 'None' to " \
                               "remove the heading.")
        settings['heading-level'] = (2, "Heading level for section title.")
        return settings

    def createHeading(self, parent, page, settings):
        heading = settings['heading']
        if heading is not None:
            h = core.Heading(parent, level=int(settings['heading-level']), id_=settings['id'])
            self.reader.tokenize(h, heading, page, MarkdownReader.INLINE)

class SyntaxDescriptionCommand(SyntaxCommandBase):
    SUBCOMMAND = 'description'
    NODE_TYPE = moosesyntax.ObjectNodeBase

    def createTokenFromSyntax(self, parent, info, page, obj, settings):

        if obj.description is None:
            msg = "The class description is missing for %s, it can be added using the " \
                  "'addClassDescription' method from within the objects validParams function."
            LOG.warning(msg, obj.fullpath())
            core.Paragraph(parent, string=str(info[0]), class_='moose-error')
            return parent

        else:
            p = core.Paragraph(parent)
            self.reader.tokenize(p, str(obj.description), page, MarkdownReader.INLINE)
            return parent

class SyntaxParametersCommand(SyntaxCommandHeadingBase):
    SUBCOMMAND = 'parameters'
    NODE_TYPE = None # allows SyntaxNode objects to report combined action parameters

    @staticmethod
    def defaultSettings():
        settings = SyntaxCommandHeadingBase.defaultSettings()
        settings['groups'] = (None, "Space separated list of groups, in desired order, to output.")
        settings['hide'] = (None, "Space separated list of parameters to remove from output.")
        settings['show'] = (None, "Space separated list of parameters to display in output.")
        settings['visible'] = (None,
                               "Space separated list of parameter groups to display with " \
                               "un-collapsed sections.")
        return settings

    def createTokenFromSyntax(self, parent, info, page, obj, settings):

        parameters = list()
        if isinstance(obj, moosesyntax.SyntaxNode):
            for action in obj.actions():
                for param in action.parameters.values():
                    parameters.append(ParameterToken(None, parameter=param, syntax=obj.name))
        elif obj.parameters:
            for param in obj.parameters.values():
                parameters.append(ParameterToken(None, parameter=param, syntax=obj.name))

        self.createHeading(parent, page, settings)
        token = InputParametersToken(parent, syntax=obj.name, parameters=parameters,
                                     **self.attributes(settings))
        if settings['groups']:
            token['groups'] = [group.strip() for group in settings['groups'].split(' ')]

        if settings['hide']:
            token['hide'] = [param.strip() for param in settings['hide'].split(' ')]

        if settings['show']:
            token['show'] = [param.strip() for param in settings['show'].split(' ')]

        if settings['visible'] is not None:
            token['visible'] = [group.strip().lower() for group in \
                                settings['visible'].split(' ')]
        else:
            token['visible'] = self.extension.get('visible')

        return parent

class SyntaxParameterCommand(SyntaxCommandBase):
    COMMAND = 'param'
    SUBCOMMAND = None
    NODE_TYPE = None

    @staticmethod
    def defaultSettings():
        settings = SyntaxCommandBase.defaultSettings()
        return settings

    def createDisabledToken(self, parent, info, page, settings):
        tag = 'span' if MarkdownReader.INLINE in info else 'p'
        tokens.DisabledToken(parent, tag=tag, string=settings['_param'])
        return parent

    def createToken(self, parent, info, page, settings):
        obj_syntax, param_name = info[MarkdownReader.INLINE].rsplit('/', 1)
        settings['syntax'] = obj_syntax
        settings['_param'] = param_name
        return SyntaxCommandBase.createToken(self, parent, info, page, settings)

    def createTokenFromSyntax(self, parent, info, page, obj, settings):
        parameters = dict()
        if isinstance(obj, moosesyntax.SyntaxNode):
            for action in obj.actions():
                parameters.update(action.parameters)
        elif obj.parameters:
            parameters.update(obj.parameters)

        param_name = settings['_param']
        if param_name not in parameters:
            obj_syntax = settings['syntax']
            results = mooseutils.levenshteinDistance(param_name, parameters.keys(), 5)
            msg = "Unable to locate the parameter '{}/{}', did you mean:\n".format(obj_syntax, param_name)
            for res in results:
                msg += '    {}/{}\n'.format(obj_syntax, res)
            raise exceptions.MooseDocsException(msg, param_name, obj_syntax)

        param_token = ParameterToken(None, parameter=parameters[param_name], syntax=obj.name, include_heading=False);
        modal.ModalLink(parent, string='"{}"'.format(param_name), title=param_name, content=param_token)
        return parent

class SyntaxChildrenCommand(SyntaxCommandHeadingBase):
    SUBCOMMAND = 'children'
    NODE_TYPE = moosesyntax.ObjectNodeBase

    @staticmethod
    def defaultSettings():
        settings = SyntaxCommandHeadingBase.defaultSettings()
        settings['heading'] = ("Child Objects",
                               "Heading to include for sections, use 'None' to remove the title.")
        return settings

    def createTokenFromSyntax(self, parent, info, page, obj, settings):

        item = self.extension.database.get(obj.name, None)
        attr = getattr(item, self.SUBCOMMAND, None)
        if item and attr:
            self.createHeading(parent, page, settings)
            ul = core.UnorderedList(parent, class_='moose-list-{}'.format(self.SUBCOMMAND))
            for filename in attr:
                filename = os.path.abspath(os.path.join(MooseDocs.ROOT_DIR, str(filename)))
                li = core.ListItem(ul)
                modal.ModalSourceLink(li, src=filename)

        return parent

class SyntaxInputsCommand(SyntaxChildrenCommand):
    SUBCOMMAND = 'inputs'
    NODE_TYPE = moosesyntax.ObjectNodeBase

    @staticmethod
    def defaultSettings():
        settings = SyntaxChildrenCommand.defaultSettings()
        settings['heading'] = ("Input Files", settings['heading'][1])
        return settings


class SyntaxListCommand(SyntaxCommandHeadingBase):
    SUBCOMMAND = 'list'
    NODE_TYPE = moosesyntax.SyntaxNode

    @staticmethod
    def defaultSettings():
        settings = SyntaxCommandHeadingBase.defaultSettings()
        settings['heading'] = ('AUTO',
                               "The heading title for the input parameters table, use 'None' to " \
                               "remove the heading.")
        settings['group-headings'] = (True, "Display group headings.")

        settings['groups'] = (None, "List of groups (apps) to include in the complete syntax list.")
        settings['actions'] = (True, "Include a list of Action objects in syntax.")
        settings['objects'] = (True, "Include a list of MooseObject objects in syntax.")
        settings['subsystems'] = (True, "Include a list of sub system syntax in the output.")
        return settings

    def createTokenFromSyntax(self, parent, info, page, obj, settings):

        primary = SyntaxList(None, **self.attributes(settings))
        if settings['groups']:
            groups = settings['groups'].split()
        else:
            groups = list(set([child.group for child in obj if child.group is not None]))

        if 'MooseApp' in groups:
            groups.remove('MooseApp')
            groups.insert(0, 'MooseApp')

        for group in groups:
            if settings['group-headings']:
                header = SyntaxListItem(primary,
                                        header=True,
                                        string=str(mooseutils.camel_to_space(group)))

            count = 0
            if settings['actions']:
                count += self._addItems(primary, info, page, group, obj.actions(), 'Action')
            if settings['objects']:
                count += self._addItems(primary, info, page, group, obj.objects(), 'MooseObject')
            if settings['subsystems']:
                count += self._addItems(primary, info, page, group, obj.syntax())

            if count == 0:
                header.parent = None

        if primary.children:
            self.createHeading(parent, page, settings)
            primary.parent = parent

        return parent

    def createHeading(self, parent, page, settings, **kwargs):
        if settings['heading'] == 'AUTO':
            h = ['Objects', 'Actions', 'Subsystems']
            idx = [settings['objects'], settings['actions'], settings['subsystems']]
            names = [h[i] for i, v in enumerate(idx) if v]
            if len(names) == 1:
                settings['heading'] = 'Available {}'.format(*names)
            elif len(names) == 2:
                settings['heading'] = 'Available {} and {}'.format(*names)
            elif len(names) == 3:
                settings['heading'] = 'Available {}, {}, and {}'.format(*names)
            else:
                settings['heading'] = None

        super(SyntaxListCommand, self).createHeading(parent, page, settings)

    def _addItems(self, parent, info, page, group, objects, base=None):

        count = 0
        for obj in objects:
            if (group in obj.groups()) and not (obj.removed or obj.test):
                count += 1
                item = SyntaxListItem(parent, group=group, syntax=obj.name)
                if base:
                    item['base'] = base
                nodes = self.translator.findPages(obj.markdown)
                if len(nodes) == 0:
                    tokens.String(item, content=str(obj.name))
                else:
                    SyntaxLink(item, string=str(obj.name),
                               url=str(nodes[0].relativeDestination(page)))

                desc = getattr(obj, 'description', None)
                if desc:
                    self.reader.tokenize(item, str(obj.description), page, MarkdownReader.INLINE, info.line)

        return count

class SyntaxCompleteCommand(SyntaxListCommand):
    SUBCOMMAND = 'complete'
    NODE_TYPE = moosesyntax.SyntaxNode

    @staticmethod
    def defaultSettings():
        settings = SyntaxListCommand.defaultSettings()
        settings['level'] = (2, "Beginning heading level.")
        settings['heading'] = (None, settings['heading'][1])
        return settings

    def createTokenFromSyntax(self, parent, info, page, obj, settings):
        # Always search entire syntax tree for group members when listing root systems by setting
        # `recursive=True`. This is so that top level headers are always rendered even if a system
        # only has actions and/or subsystems available for the group and no child objects.
        self._addList(parent, info, page, obj, settings, level=settings['level'], recursive=True)
        return parent

    def _addList(self, parent, info, page, obj, settings, *, level=None, recursive=False):
        groups = set(settings['groups'].split()) if settings['groups'] else None
        h_id = settings['id']
        for child in obj.syntax():
            if child.removed or child.test:
                continue

            # get a list of groups this object is a member of to cross-reference w/ those specified
            cgs = child.groups(actions=True, syntax=True, objects=True, recursive=recursive)

            if (groups is None) or (cgs.intersection(groups)):
                url = os.path.join('syntax', child.markdown)
                h = core.Heading(parent, level=level, id_=h_id)
                autolink.AutoLink(h, page=url, string=str(child.fullpath().strip('/')))

            SyntaxListCommand.createTokenFromSyntax(self, parent, info, page, child, settings)

            self._addList(parent, info, page, child, settings, level=level+1)


class RenderSyntaxList(components.RenderComponent):
    def createHTML(self, parent, token, page):
        div = html.Tag(parent, 'div', token, class_='moose-syntax-list')
        return div

    def createMaterialize(self, parent, token, page):
        collection = html.Tag(parent, 'ul', class_='moose-syntax-list collection with-header')
        return collection

    def createLatex(self, parent, token, page):
        return parent

    def createReveal(self, parent, token, page):
        div = self.createHTML(parent, token, page)

        arrow_div = html.Tag(div, 'div', class_='moose-arrow moose-bounce')
        arrow_i = html.Tag(arrow_div, 'i', class_='moose-scroll-indicator',
                           string='keyboard_arrow_down')
        arrow_i.addClass('material-icons')
        arrow_i['aria-hidden'] = "true"

        return div

class RenderSyntaxListItem(components.RenderComponent):
    def createHTML(self, parent, token, page):
        #token(0).parent = None # I don't recall why this was here
        p = html.Tag(parent, 'p', class_='moose-syntax-list-item')
        html.Tag(p, 'span', string='{}: '.format(token['syntax']),
                 class_='moose-syntax-list-item-syntax')
        return html.Tag(p, 'span',
                        class_='moose-syntax-list-item-details')

    def createMaterialize(self, parent, token, page):
        class_ = 'collection-header' if token['header'] else 'collection-item'
        return html.Tag(parent, 'li', class_=class_)

    def createLatex(self, parent, token, page):
        if token['header']:
            return None

        title = latex.Brace(string=token(0)['content'])
        reg = latex.Bracket(string=token['group'])
        args = [title, reg]

        if token['base']:
            args.append(latex.Bracket(string=token['base']))

        token(0).parent = None
        env = latex.Environment(parent, 'ObjectDescription', args=args)
        if len(token) == 0:
            latex.String(env, content="\\textcolor{red}{No Description.}", escape=False)
        return env

class RenderInputParametersToken(components.RenderComponent):

    def createHTML(self, parent, token, page):
        self._createHTMLHelper(parent, token, page, False)

    def createMaterialize(self, parent, token, page):
        self._createHTMLHelper(parent, token, page, True)

    def _createHTMLHelper(self, parent, token, page, collapse):
        groups = _get_parameters(token, token['parameters'])

        n_groups = 0
        for group, params in groups.items():
            if len(params):
                n_groups += 1

        for group, params in groups.items():
            if not params:
                continue

            if n_groups > 1: # only create a sub-section if more than one exists
                h = html.Tag(parent, 'h{}'.format(token['level'] + 1),
                             string=str('{} Parameters'.format(group.title())))
                if group.lower() in token['visible']:
                    h['data-details-open'] = 'open'
                else:
                    h['data-details-open'] = 'close'

            ul = html.Tag(parent, 'ul')
            if collapse:
                ul.addClass('collapsible')
                ul['data-collapsible'] = "expandable"

            for name, param in params.items():
                self.renderer.render(ul, param, page)
                # func(ul, name, param)

    def createLatex(self, parent, token, page):

        groups = _get_parameters(token, token['parameters'])
        for group, params in groups.items():
            if not params:
                continue

            for name, param in params.items():
                self.renderer.render(parent, param, page)

class RenderParameterToken(components.RenderComponent):

    def createHTML(self, parent, token, page):
        param = token['parameter']
        name = param['name']

        if param['deprecated']:
            return

        li = html.Tag(parent, 'li')
        default = _format_default(param)

        if default:
            html.Tag(li, 'strong', string=name)
            html.Tag(li, 'span', string=' ({}): '.format(default))
        else:
            html.Tag(li, 'strong', string='{}: '.format(name))

        desc = param['description']
        if desc:
            html.Tag(li, 'span', string=desc)

    def createMaterialize(self, parent, token, page):
        param = token['parameter']
        name = param['name']

        if param['deprecated']:
            return

        default = _format_default(param)
        desc = param['description']

        if token['include_heading']:
            li = html.Tag(parent, 'li')
            header = html.Tag(li, 'div', class_='collapsible-header')
            body = html.Tag(li, 'div', class_='collapsible-body')

            html.Tag(header, 'span', class_='moose-parameter-name', string=name)
            if default:
                html.Tag(header, 'span', class_='moose-parameter-header-default', string=default)

            if desc:
                html.Tag(header, 'span', class_='moose-parameter-header-description', string=str(desc))

        else:
            body = parent

        if default:
            p = html.Tag(body, 'p', class_='moose-parameter-description-default')
            html.Tag(p, 'span', string='Default:')
            html.String(p, content=default)

        cpp_type = param['cpp_type']
        p = html.Tag(body, 'p', class_='moose-parameter-description-cpptype')
        html.Tag(p, 'span', string='C++ Type:')
        html.String(p, content=cpp_type, escape=True)

        if param['options']:
            p = html.Tag(body, 'p', class_='moose-parameter-description-options')
            html.Tag(p, 'span', string='Options:')
            html.String(p, content=", ".join(param['options'].split()))

        p = html.Tag(body, 'p', class_='moose-parameter-description-controllable')
        html.Tag(p, 'span', string='Controllable:')
        html.String(p, content=('Yes' if param['controllable'] else 'No'))

        p = html.Tag(body, 'p', class_='moose-parameter-description')
        if desc:
            html.Tag(p, 'span', string='Description:')
            html.String(p, content=str(desc))

    def createLatex(self, parent, token, page):
        param = token['parameter']
        name = param['name']
        group = param['group_name']

        if param['deprecated']:
            return

        if not group and param['required']:
            group = 'Required'
        elif not group and not param['required']:
            group = 'Optional'

        args = [latex.Brace(string=name), latex.Bracket(string=group), latex.Bracket()]
        latex.Command(args[2], 'texttt', string=param['cpp_type'])
        default = _format_default(param) or ''
        if default:
            args.append(latex.Bracket(string=default))

        latex.Environment(parent, 'InputParameter',
                          args=args,
                          string=param['description'])

class RenderSyntaxLink(core.RenderLink):
    def createLatex(self, parent, token, page):
        return parent

def _get_parameters(token, parameters):
    """
    Add the parameters from the supplied node to the supplied groups
    """

    # Build the list of groups to display
    groups = collections.OrderedDict()
    if token['groups']:
        for group in token['groups']:
            groups[group] = dict()

    else:
        groups['Required'] = dict()
        groups['Optional'] = dict()
        for param in parameters:
            group = param['parameter']['group_name']
            if group and group not in groups:
                groups[group] = dict()

    # Populate the parameter lists by group
    for param in parameters or []:

        # Do nothing if the parameter is hidden or not shown
        name = param['parameter']['name']
        if (name == 'type') or \
           (token['hide'] and name in token['hide']) or \
           (token['show'] and name not in token['show']):
            continue

        # Handle the 'ungroup' parameters
        group = param['parameter']['group_name']
        if not group and param['parameter']['required']:
            group = 'Required'
        elif not group and not param['parameter']['required']:
            group = 'Optional'

        if group in groups:
            groups[group][name] = param

    return groups

def _format_default(parameter):
    """
    Convert the supplied parameter into a format suitable for output.

    Args:
        parameter[str]: The parameter dict() item.
        key[str]: The current key.
    """

    ptype = parameter['cpp_type']
    param = parameter.get('default', '')

    if ptype == 'bool':
        param = repr(param in ['True', '1'])

    return str(param) if param else None
