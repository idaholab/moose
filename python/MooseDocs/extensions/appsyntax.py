#pylint: disable=missing-docstring,attribute-defined-outside-init
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
import anytree

import mooseutils

import MooseDocs
from MooseDocs import common
from MooseDocs.common import exceptions
from MooseDocs.base import components, LatexRenderer
from MooseDocs.tree import html, tokens, syntax, latex, app_syntax
from MooseDocs.extensions import core, floats, table, autolink, materialicon

from MooseDocs.extensions import command

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    return AppSyntaxExtension(**kwargs)

InputParametersToken = tokens.newToken('InputParametersToken',
                                       parameters=dict(),
                                       level=2,
                                       groups=list(),
                                       hide=set(),
                                       show=set(),
                                       visible=set())
ParameterToken = tokens.newToken('ParameterToken',
                                 inline=False,
                                 parameter=None,
                                 description=True,
                                 default=True,
                                 options=True,
                                 cpp_type=True)
SyntaxList = tokens.newToken('SyntaxList')
SyntaxListItem = tokens.newToken('SyntaxListItem', syntax=u'', group=u'', header=False)
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
        config['includes'] = ([],
                              "List of include directories to investigate for class information.")
        config['inputs'] = ([],
                            "List of directories to interrogate for input files using an object.")
        config['hide'] = (None, "Dictionary of syntax to hide.")
        config['remove'] = (None, "List or Dictionary of lists of syntax to remove.")
        config['visible'] = (set(['required', 'optional']),
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

    def preExecute(self, content):
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
        exe = mooseutils.find_moose_executable(exe, show_error=False)

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

            except Exception as e: #pylint: disable=broad-except
                msg = "Failed to load application executable from '%s', " \
                      "application syntax is being disabled:\n%s"
                self.setActive(False)
                LOG.error(msg, self.get('executable'), e.message)
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
        for node in anytree.PreOrderIter(self._app_syntax):
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
        try:
            if node_type == syntax.ObjectNode:
                return self._object_cache[name]
            elif node_type == syntax.SyntaxNode:
                return self._syntax_cache[name]
            return self._cache[name]
        except KeyError:
            msg = "'{}' syntax was not recognized."
            raise common.MooseDocsException(msg, name)

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
        settings['heading'] = (u'Input Parameters',
                               "The heading title for the input parameters table, use 'None' to " \
                               "remove the heading.")
        settings['heading-level'] = (2, "Heading level for section title.")
        return settings

    def createHeading(self, parent, page, settings=None):
        settings = settings or self.settings
        heading = settings['heading']
        if heading is not None:
            h = core.Heading(parent, level=int(settings['heading-level']))
            self.reader.tokenize(h, heading, page, MooseDocs.INLINE)

class SyntaxDescriptionCommand(SyntaxCommandBase):
    SUBCOMMAND = 'description'
    NODE_TYPE = syntax.ObjectNode

    def createTokenFromSyntax(self, parent, info, page, obj):

        if obj.description is None:
            if not obj.hidden:
                msg = "The class description is missing for {}, it can be added using the " \
                      "'addClassDescription' method from within the objects validParams function."
                raise exceptions.MooseDocsException(msg, obj.fullpath)
            else:
                core.Paragraph(parent, string=unicode(info[0]), class_='moose-error')
                return parent

        else:
            p = core.Paragraph(parent)
            self.reader.tokenize(p, unicode(obj.description), page, MooseDocs.INLINE)
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
            token['hide'] = set([param.strip() for param in self.settings['hide'].split(' ')])

        if self.settings['show']:
            token['show'] = set([param.strip() for param in self.settings['show'].split(' ')])

        if self.settings['visible'] is not None:
            token['visible'] = set([group.strip().lower() for group in \
                                    self.settings['visible'].split(' ')])
        else:
            token['visible'] = self.extension.get('visible')

        return parent

class SyntaxParameterCommand(SyntaxCommandHeadingBase):
    SUBCOMMAND = 'parameter'
    NODE_TYPE = None # allows SyntaxNode objects to report combined action parameters

    @staticmethod
    def defaultSettings():
        settings = SyntaxCommandHeadingBase.defaultSettings()
        settings['name'] = (None, "The name of the parameter to display.")
        settings['description'] = (True, "Show the description.")
        settings['default'] = (True, "Show the default value.")
        settings['cpptype'] = (True, "Show the C++ type.")
        settings['options'] = (True, "Show the available options.")
        return settings

    def createTokenFromSyntax(self, parent, info, page, obj):

        parameters = dict()
        if isinstance(obj, syntax.SyntaxNode):
            for action in obj.actions():
                parameters.update(action.parameters)
        elif obj.parameters:
            parameters.update(obj.parameters)

        return ParameterToken(parent,
                              description=self.settings['description'],
                              default=self.settings['default'],
                              cpp_type=self.settings['cpptype'],
                              options=self.settings['options'],
                              parameter=parameters[self.settings['name']])


class SyntaxChildrenCommand(SyntaxCommandHeadingBase):
    SUBCOMMAND = 'children'
    NODE_TYPE = syntax.ObjectNode

    @staticmethod
    def defaultSettings():
        settings = SyntaxCommandHeadingBase.defaultSettings()
        settings['heading'] = (u"Child Objects",
                               "Heading to include for sections, use 'None' to remove the title.")
        return settings

    def createTokenFromSyntax(self, parent, info, page, obj):

        item = self.extension.database.get(obj.name, None)
        attr = getattr(item, self.SUBCOMMAND, None)
        if item and attr:
            self.createHeading(parent, page)
            ul = core.UnorderedList(parent, class_='moose-list-{}'.format(self.SUBCOMMAND))
            for filename in attr:
                filename = unicode(filename)
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
        settings['heading'] = (u"Input Files", settings['heading'][1])
        return settings


class SyntaxListCommand(SyntaxCommandHeadingBase):
    SUBCOMMAND = 'list'
    NODE_TYPE = syntax.SyntaxNode

    @staticmethod
    def defaultSettings():
        settings = SyntaxCommandHeadingBase.defaultSettings()
        settings['heading'] = (u'AUTO',
                               "The heading title for the input parameters table, use 'None' to " \
                               "remove the heading.")
        settings['group-headings'] = (True, "Display group headings.")

        settings['groups'] = (None, "List of groups (apps) to include in the complete syntax list.")
        settings['actions'] = (True, "Include a list of Action objects in syntax.")
        settings['objects'] = (True, "Include a list of MooseObject objects in syntax.")
        settings['subsystems'] = (True, "Include a list of sub system syntax in the output.")
        return settings

    def createTokenFromSyntax(self, parent, info, page, obj):

        master = SyntaxList(None, **self.attributes)

        groups = self.settings['groups'].split() if self.settings['groups'] else list(obj.groups)
        if 'MooseApp' in groups:
            groups.remove('MooseApp')
            groups.insert(0, 'MooseApp')

        for group in groups:
            if self.settings['group-headings']:
                header = SyntaxListItem(master,
                                        header=True,
                                        string=unicode(mooseutils.camel_to_space(group)))

            count = 0
            if self.settings['actions']:
                count += self._addItems(master, info, page, group, obj.actions(), 'Action')
            if self.settings['objects']:
                count += self._addItems(master, info, page, group, obj.objects(), 'MooseObject')
            if self.settings['subsystems']:
                count += self._addItems(master, info, page, group, obj.syntax())

            if count == 0:
                header.parent = None

        if master.children:
            self.createHeading(parent, page)
            master.parent = parent

        return parent

    def createHeading(self, parent, page, **kwargs):
        if self.settings['heading'] == u'AUTO':
            h = ['Objects', 'Actions', 'Subsystems']
            idx = [self.settings['objects'], self.settings['actions'], self.settings['subsystems']]
            names = [h[i] for i, v in enumerate(idx) if v]
            if len(names) == 1:
                self.settings['heading'] = u'Available {}'.format(*names)
            elif len(names) == 2:
                self.settings['heading'] = u'Available {} and {}'.format(*names)
            elif len(names) == 3:
                self.settings['heading'] = u'Available {}, {}, and {}'.format(*names)
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
                    tokens.String(item, content=unicode(obj.name))
                else:
                    SyntaxLink(item, string=unicode(obj.name),
                               url=unicode(nodes[0].relativeDestination(page)))

                if obj.description:
                    self.reader.tokenize(item, obj.description, page, MooseDocs.INLINE, info.line)

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
                h = core.Heading(parent, level=level)
                autolink.AutoLink(h, page=url, string=unicode(child.fullpath.strip('/')))

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
        token(0).parent = None
        p = html.Tag(parent, 'p', class_='moose-syntax-list-item')
        html.Tag(p, 'span', string=u'{}: '.format(token['syntax']),
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
        pass

    def createMaterialize(self, parent, token, page):

        groups = self._getParameters(token, token['parameters'])
        for group, params in groups.iteritems():

            if not params:
                continue

            if len(groups) > 1: # only create a sub-section if more than one exists
                h = html.Tag(parent, 'h{}'.format(token['level'] + 1),
                             string=unicode('{} Parameters'.format(group.title())))
                if group.lower() in token['visible']:
                    h['data-details-open'] = 'open'
                else:
                    h['data-details-open'] = 'close'

            ul = html.Tag(parent, 'ul', class_='collapsible')
            ul['data-collapsible'] = "expandable"

            for name, param in params.iteritems():
                _insert_parameter(ul, name, param)

        return parent

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
            for param in parameters.itervalues():
                group = param['group_name']
                if group and group not in groups:
                    groups[group] = dict()

        # Populate the parameter lists by group
        for param in parameters.itervalues() or []:

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
        for group, params in groups.iteritems():
            if not params:
                continue

            for name, param in params.iteritems():
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
        param = token['parameter']
        if token['inline']:
            html.Tag(parent, 'span', string=param['name'], class_='moose-parameter-name')
        else:
            div = html.Tag(parent, 'div', class_='moose-parameter')
            html.Tag(div, 'span', string=param['name'], class_='moose-parameter-name')
            self._addParamInfo(parent, token, 'description', param)
            self._addParamInfo(parent, token, 'default', param)
            self._addParamInfo(parent, token, 'cpp_type', param, title='C++ Type')
            self._addParamInfo(parent, token, 'options', param)
        return parent

    def createMaterialize(self, parent, token, page):
        pass

    def createLatex(self, parent, token, page):
        pass

    @staticmethod
    def _addParamInfo(parent, token, key, param, title=None):

        value = param.get(key, None)
        if value and token[key]:
            key = key.replace('_', '')
            title = key.title() if title is None else title
            div = html.Tag(parent, 'div', class_='moose-parameter-{}'.format(key))
            html.Tag(div, 'span', string='{}: '.format(title),
                     class_='moose-parameter-{}-title'.format(key))
            html.Tag(div, 'span', string=value,
                     class_='moose-parameter-{}-content'.format(key))


class RenderSyntaxLink(core.RenderLink):
    def createLatex(self, parent, token, page):
        return parent


def _insert_parameter(parent, name, param):
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
        html.Tag(p, 'span', string=u'Default:')
        html.String(p, content=default)

    cpp_type = param['cpp_type']
    p = html.Tag(body, 'p', class_='moose-parameter-description-cpptype')
    html.Tag(p, 'span', string=u'C++ Type:')
    html.String(p, content=cpp_type)

    if 'options' in param:
        p = html.Tag(body, 'p', class_='moose-parameter-description-options')
        html.Tag(p, 'span', string=u'Options:')
        html.String(p, content=param['options'])

    p = html.Tag(body, 'p', class_='moose-parameter-description')
    desc = param['description']
    if desc:
        html.Tag(header, 'span', class_='moose-parameter-header-description', string=unicode(desc))
        html.Tag(p, 'span', string=u'Description:')
        html.String(p, content=unicode(desc))

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

    return unicode(param) if param else None
