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
import moosetree

import mooseutils

import MooseDocs
from .. import common
from ..common import exceptions
from ..base import components, LatexRenderer, MarkdownReader
from ..tree import html, tokens, syntax, latex, app_syntax
from . import command, core, floats, table, autolink, materialicon

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    return AppSyntaxExtension(**kwargs)

ParameterToken = tokens.newToken('ParameterToken', parameter=None)
InputParametersToken = tokens.newToken('InputParametersToken',
                                       parameters=dict(),
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

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['executable'] = (None,
                                "The MOOSE application executable to use for generating syntax.")
        config['app_name'] = (None,
                              "The MOOSE application name (e.g., moose_test); if not provided an " \
                              "attempt will be made to determine the name.")
        config['includes'] = ([],
                              "List of include directories to investigate for class information.")
        config['inputs'] = ([],
                            "List of directories to interrogate for input files using an object.")
        config['hide'] = (None, "Dictionary of syntax to hide.")
        config['remove'] = (None, "List or Dictionary of lists of syntax to remove.")
        config['visible'] = (['required', 'optional'],
                             "Parameter groups to show as un-collapsed.")
        config['alias'] = (None, "List of Dictionary of lists of syntax aliases.")
        config['allow-test-objects'] = (False, "Enable the test objects.")
        return config

    def __init__(self, *args, **kwargs):
        command.CommandExtension.__init__(self, *args, **kwargs)

        self._app_type = None
        self._app_syntax = None
        self._database = None
        self._cache = dict()
        self._object_cache = dict()
        self._syntax_cache = dict()

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

        if exe is None:
            LOG.error("Failed to locate a valid executable in %s.", self['executable'])
        else:
            try:
                self._app_syntax = app_syntax(exe,
                                              alias=self['alias'],
                                              remove=self['remove'],
                                              hide=self['hide'],
                                              allow_test_objects=self['allow-test-objects'])

                out = mooseutils.runExe(exe, ['--type'])
                match = re.search(r'^MooseApp Type:\s+(?P<type>.*?)$', out, flags=re.MULTILINE)
                if match:
                    self._app_type = match.group("type")
                else:
                    msg = "Failed to determine application type by running the following:\n"
                    msg += "    {} --type".format(exe)
                    LOG.error(msg)

            except Exception as e:
                msg = "Failed to load application executable from '%s', " \
                      "application syntax is being disabled:\n%s"
                self.setActive(False)
                LOG.error(msg, exe, e)
        LOG.info("MOOSE application syntax complete [%s sec.]", time.time() - start)

    def __initClassDatabase(self):
        """Initialize the class database for faster searching."""

        # Do nothing if the syntax failed to build
        if self._app_syntax is None:
            return

        start = time.time()
        LOG.info("Building MOOSE class database...")
        self._database = common.build_class_database(self['includes'], self['inputs'])

        # Cache the syntax entries, search the tree is very slow
        self._cache = dict()
        self._object_cache = dict()
        self._syntax_cache = dict()
        for node in moosetree.iterate(self._app_syntax):
            if not node.removed:
                self._cache[node.fullpath] = node
                if node.alias:
                    self._cache[node.alias] = node
                if isinstance(node, syntax.ObjectNode):
                    self._object_cache[node.fullpath] = node
                    if node.alias:
                        self._object_cache[node.alias] = node
                elif isinstance(node, syntax.SyntaxNode):
                    self._syntax_cache[node.fullpath] = node
                    if node.alias:
                        self._syntax_cache[node.alias] = node
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

    def find(self, name, node_type=None):

        if name.endswith('<RESIDUAL>'):
            msg = "The use of <RESIDUAL> is no longer needed in the syntax name '%s', it " \
                  "should be removed."
            LOG.warning(msg, name)
            name = name[0:-10]

        node = None
        if node_type == syntax.ObjectNode:
            node = self._object_cache.get(name, None)
        elif node_type == syntax.SyntaxNode:
            node = self._syntax_cache.get(name, None)
        else:
            node = self._cache.get(name, None)

        if node is None:
            msg = "'{}' syntax was not recognized."
            raise exceptions.MooseDocsException(msg, name)

        return node

    def extend(self, reader, renderer):
        self.requires(core, floats, table, autolink, materialicon)

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

    def createToken(self, parent, info, page):
        if self.settings['syntax'] is None:
            args = info['settings'].split()
            if args and ('=' not in args[0]):
                self.settings['syntax'] = args[0]

        if self.settings['syntax']:
            obj = self.extension.find(self.settings['syntax'], self.NODE_TYPE)
        else:
            obj = self.extension.syntax

        return self.createTokenFromSyntax(parent, info, page, obj)

    def createTokenFromSyntax(self, parent, info, page, obj):
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

    def createHeading(self, parent, page, settings=None):
        settings = settings or self.settings
        heading = settings['heading']
        if heading is not None:
            h = core.Heading(parent, level=int(settings['heading-level']), id_=settings['id'])
            self.reader.tokenize(h, heading, page, MarkdownReader.INLINE)

class SyntaxDescriptionCommand(SyntaxCommandBase):
    SUBCOMMAND = 'description'
    NODE_TYPE = syntax.ObjectNode

    def createTokenFromSyntax(self, parent, info, page, obj):

        if obj.description is None:
            msg = "The class description is missing for %s, it can be added using the " \
                  "'addClassDescription' method from within the objects validParams function."
            if not obj.hidden:
                LOG.warning(msg, obj.fullpath)
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

    def createTokenFromSyntax(self, parent, info, page, obj):

        parameters = dict()
        if isinstance(obj, syntax.SyntaxNode):
            for action in obj.actions():
                parameters.update(action.parameters)
        elif obj.parameters:
            parameters.update(obj.parameters)

        self.createHeading(parent, page)
        token = InputParametersToken(parent, syntax=obj.name, parameters=parameters,
                                     **self.attributes)
        if self.settings['groups']:
            token['groups'] = [group.strip() for group in self.settings['groups'].split(' ')]

        if self.settings['hide']:
            token['hide'] = [param.strip() for param in self.settings['hide'].split(' ')]

        if self.settings['show']:
            token['show'] = [param.strip() for param in self.settings['show'].split(' ')]

        if self.settings['visible'] is not None:
            token['visible'] = [group.strip().lower() for group in \
                                self.settings['visible'].split(' ')]
        else:
            token['visible'] = self.extension.get('visible')

        return parent

class SyntaxParameterCommand(command.CommandComponent):
    COMMAND = 'param'
    SUBCOMMAND = None

    @staticmethod
    def defaultSettings():
        settings = SyntaxCommandHeadingBase.defaultSettings()
        return settings

    def createToken(self, parent, info, page):

        obj_syntax, param_name = info[MarkdownReader.INLINE].rsplit('/', 1)

        obj = self.extension.find(obj_syntax)
        parameters = dict()
        if isinstance(obj, syntax.SyntaxNode):
            for action in obj.actions():
                parameters.update(action.parameters)
        elif obj.parameters:
            parameters.update(obj.parameters)

        if param_name not in parameters:
            results = mooseutils.levenshteinDistance(param_name, parameters.keys(), 5)
            msg = "Unable to locate the parameter '{}/{}', did you mean:\n"
            for res in results:
                msg += '    {}/{}\n'.format(obj_syntax, res)
            raise exceptions.MooseDocsException(msg, param_name, obj_syntax)

        ParameterToken(parent, parameter=parameters[param_name],
                       string='"{}"'.format(param_name))
        return parent


class SyntaxChildrenCommand(SyntaxCommandHeadingBase):
    SUBCOMMAND = 'children'
    NODE_TYPE = syntax.ObjectNode

    @staticmethod
    def defaultSettings():
        settings = SyntaxCommandHeadingBase.defaultSettings()
        settings['heading'] = ("Child Objects",
                               "Heading to include for sections, use 'None' to remove the title.")
        return settings

    def createTokenFromSyntax(self, parent, info, page, obj):

        item = self.extension.database.get(obj.name, None)
        attr = getattr(item, self.SUBCOMMAND, None)
        if item and attr:
            self.createHeading(parent, page)
            ul = core.UnorderedList(parent, class_='moose-list-{}'.format(self.SUBCOMMAND))
            for filename in attr:
                filename = str(filename)
                li = core.ListItem(ul)
                lang = common.get_language(filename)
                content = common.fix_moose_header(common.read(os.path.join(MooseDocs.ROOT_DIR,
                                                                           filename)))
                code = core.Code(None, language=lang, content=content)
                link = floats.create_modal_link(li,
                                                url=filename,
                                                content=code,
                                                title=filename,
                                                string=filename)
                link.name = 'SyntaxLink'
        return parent

class SyntaxInputsCommand(SyntaxChildrenCommand):
    SUBCOMMAND = 'inputs'
    NODE_TYPE = syntax.ObjectNode

    @staticmethod
    def defaultSettings():
        settings = SyntaxChildrenCommand.defaultSettings()
        settings['heading'] = ("Input Files", settings['heading'][1])
        return settings


class SyntaxListCommand(SyntaxCommandHeadingBase):
    SUBCOMMAND = 'list'
    NODE_TYPE = syntax.SyntaxNode

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

    def createTokenFromSyntax(self, parent, info, page, obj):

        primary = SyntaxList(None, **self.attributes)

        groups = self.settings['groups'].split() if self.settings['groups'] else list(obj.groups)
        if 'MooseApp' in groups:
            groups.remove('MooseApp')
            groups.insert(0, 'MooseApp')

        for group in groups:
            if self.settings['group-headings']:
                header = SyntaxListItem(primary,
                                        header=True,
                                        string=str(mooseutils.camel_to_space(group)))

            count = 0
            if self.settings['actions']:
                count += self._addItems(primary, info, page, group, obj.actions(), 'Action')
            if self.settings['objects']:
                count += self._addItems(primary, info, page, group, obj.objects(), 'MooseObject')
            if self.settings['subsystems']:
                count += self._addItems(primary, info, page, group, obj.syntax())

            if count == 0:
                header.parent = None

        if primary.children:
            self.createHeading(parent, page)
            primary.parent = parent

        return parent

    def createHeading(self, parent, page, **kwargs):
        if self.settings['heading'] == 'AUTO':
            h = ['Objects', 'Actions', 'Subsystems']
            idx = [self.settings['objects'], self.settings['actions'], self.settings['subsystems']]
            names = [h[i] for i, v in enumerate(idx) if v]
            if len(names) == 1:
                self.settings['heading'] = 'Available {}'.format(*names)
            elif len(names) == 2:
                self.settings['heading'] = 'Available {} and {}'.format(*names)
            elif len(names) == 3:
                self.settings['heading'] = 'Available {}, {}, and {}'.format(*names)
            else:
                self.settings['heading'] = None

        super(SyntaxListCommand, self).createHeading(parent, page, copy.copy(self.settings))

    def _addItems(self, parent, info, page, group, objects, base=None):

        count = 0
        for obj in objects:
            if (group in obj.groups) and (not obj.removed):
                count += 1
                item = SyntaxListItem(parent, group=group, syntax=obj.name)
                if base:
                    item['base'] = base
                nodes = self.translator.findPages(obj.markdown())
                if len(nodes) == 0:
                    tokens.String(item, content=str(obj.name))
                else:
                    SyntaxLink(item, string=str(obj.name),
                               url=str(nodes[0].relativeDestination(page)))

                if obj.description:
                    self.reader.tokenize(item, str(obj.description), page, MarkdownReader.INLINE, info.line)

        return count

class SyntaxCompleteCommand(SyntaxListCommand):
    SUBCOMMAND = 'complete'
    NODE_TYPE = syntax.SyntaxNode

    @staticmethod
    def defaultSettings():
        settings = SyntaxListCommand.defaultSettings()
        settings['level'] = (2, "Beginning heading level.")
        settings['heading'] = (None, settings['heading'][1])
        return settings

    def createTokenFromSyntax(self, parent, info, page, obj):
        self._addList(parent, info, page, obj, self.settings['level'])
        return parent

    def _addList(self, parent, info, page, obj, level):

        gs = self.settings['groups']
        groups = set(gs.split()) if gs else set(obj.groups)

        for child in obj.syntax():
            if child.removed:
                continue

            if child.groups.intersection(groups):
                url = os.path.join('syntax', child.markdown())
                h = core.Heading(parent, level=level, id_=self.settings['id'])
                autolink.AutoLink(h, page=url, string=str(child.fullpath.strip('/')))

            SyntaxListCommand.createTokenFromSyntax(self, parent, info, page, child)
            self._addList(parent, info, page, child, level + 1)


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
        self._createHTMLHelper(parent, token, page, _insert_html_parameter, False)

    def createMaterialize(self, parent, token, page):
        self._createHTMLHelper(parent, token, page, _insert_materialize_parameter, True)

    def _createHTMLHelper(self, parent, token, page, func, collapse):
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
                func(ul, name, param)

    @staticmethod
    def _getParameters(token, parameters):
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
            for param in parameters.values():
                group = param['group_name']
                if group and group not in groups:
                    groups[group] = dict()

        # Populate the parameter lists by group
        for param in parameters.values() or []:

            # Do nothing if the parameter is hidden or not shown
            name = param['name']
            if (name == 'type') or \
               (token['hide'] and name in token['hide']) or \
               (token['show'] and name not in token['show']):
                continue

            # Handle the 'ungroup' parameters
            group = param['group_name']
            if not group and param['required']:
                group = 'Required'
            elif not group and not param['required']:
                group = 'Optional'

            if group in groups:
                groups[group][name] = param

        return groups

    def createLatex(self, parent, token, page):

        groups = self._getParameters(token, token['parameters'])
        for group, params in groups.items():
            if not params:
                continue

            for name, param in params.items():
                if param['deprecated']:
                    continue

                args = [latex.Brace(string=name), latex.Bracket(string=group), latex.Bracket()]
                latex.Command(args[2], 'texttt', string=param['cpp_type'])
                default = _format_default(param) or ''
                if default:
                    args.append(latex.Bracket(string=default))

                latex.Environment(parent, 'InputParameter',
                                  args=args,
                                  string=param['description'])

class RenderParameterToken(components.RenderComponent):

    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'span', class_='moose-parameter-name')

    def createMaterialize(self, parent, token, page):
        span = self.createHTML(parent, token, page)
        param = token['parameter']
        span.addClass('tooltipped')
        span['data-tooltip'] = param['description']
        span['data-position'] = 'bottom'
        span['data-delay'] = 5
        return span

    def createLatex(self, parent, token, page):
        return parent

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
        for param in parameters.values():
            group = param['group_name']
            if group and group not in groups:
                groups[group] = dict()

    # Populate the parameter lists by group
    for param in parameters.values() or []:

        # Do nothing if the parameter is hidden or not shown
        name = param['name']
        if (name == 'type') or \
           (token['hide'] and name in token['hide']) or \
           (token['show'] and name not in token['show']):
            continue

        # Handle the 'ungroup' parameters
        group = param['group_name']
        if not group and param['required']:
            group = 'Required'
        elif not group and not param['required']:
            group = 'Optional'

        if group in groups:
            groups[group][name] = param

    return groups

def _insert_html_parameter(parent, name, param):
    """
    Insert parameter in to the supplied <ul> tag.

    Input:
        parent[html.Tag]: The 'ul' tag that parameter <li> item is to belong.
        name[str]: The name of the parameter.
        param: The parameter object from JSON dump.
    """
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

def _insert_materialize_parameter(parent, name, param):
    """
    Insert parameter in to the supplied <ul> tag.

    Input:
        parent[html.Tag]: The 'ul' tag that parameter <li> item is to belong.
        name[str]: The name of the parameter.
        param: The parameter object from JSON dump.
    """
    if param['deprecated']:
        return

    li = html.Tag(parent, 'li')
    header = html.Tag(li, 'div', class_='collapsible-header')
    body = html.Tag(li, 'div', class_='collapsible-body')

    html.Tag(header, 'span', class_='moose-parameter-name', string=name)
    default = _format_default(param)
    if default:
        html.Tag(header, 'span', class_='moose-parameter-header-default', string=default)

        p = html.Tag(body, 'p', class_='moose-parameter-description-default')
        html.Tag(p, 'span', string='Default:')
        html.String(p, content=default)

    cpp_type = param['cpp_type']
    p = html.Tag(body, 'p', class_='moose-parameter-description-cpptype')
    html.Tag(p, 'span', string='C++ Type:')
    html.String(p, content=cpp_type)

    if 'options' in param:
        p = html.Tag(body, 'p', class_='moose-parameter-description-options')
        html.Tag(p, 'span', string='Options:')
        html.String(p, content=param['options'])

    p = html.Tag(body, 'p', class_='moose-parameter-description')
    desc = param['description']
    if desc:
        html.Tag(header, 'span', class_='moose-parameter-header-description', string=str(desc))
        html.Tag(p, 'span', string='Description:')
        html.String(p, content=str(desc))

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
