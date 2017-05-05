#pylint: disable=missing-docstring
####################################################################################################
#                                    DO NOT MODIFY THIS HEADER                                     #
#                   MOOSE - Multiphysics Object Oriented Simulation Environment                    #
#                                                                                                  #
#                              (c) 2010 Battelle Energy Alliance, LLC                              #
#                                       ALL RIGHTS RESERVED                                        #
#                                                                                                  #
#                            Prepared by Battelle Energy Alliance, LLC                             #
#                               Under Contract No. DE-AC07-05ID14517                               #
#                               With the U. S. Department of Energy                                #
#                                                                                                  #
#                               See COPYRIGHT for full restrictions                                #
####################################################################################################
#pylint: enable=missing-docstring

import os
import collections
import cPickle as pickle
import logging
import mooseutils

from markdown.util import etree
from markdown.inlinepatterns import Pattern
from MooseMarkdownExtension import MooseMarkdownExtension
from MooseMarkdownCommon import MooseMarkdownCommon
from MooseObjectParameterTable import MooseObjectParameterTable
import MooseDocs
from .. import common

LOG = logging.getLogger(__name__)

class AppSyntaxExtension(MooseMarkdownExtension):
    """
    Extensions that comprise the MOOSE flavored markdown.
    """
    @staticmethod
    def defaultConfig():
        """Default settings for MooseMarkdownCommon."""

        config = MooseMarkdownExtension.defaultConfig()
        config['executable'] = ['', "The executable to utilize for generating application syntax."]
        config['locations'] = [dict(), "The locations to parse for syntax."]
        config['repo'] = ['', "The remote repository to create hyperlinks."]
        config['links'] = [dict(), "The set of paths for generating input file and source code "
                                   "links to objects."]
        config['install'] = ['', "The location to install system and object documentation."]
        return config

    def __init__(self, **kwargs):
        super(AppSyntaxExtension, self).__init__(**kwargs)

        # Storage for the MooseLinkDatabase object
        self.syntax = None

        # Create the absolute path to the executable
        self.setConfig('executable', MooseDocs.abspath(self.getConfig('executable')))

    def execute(self):
        """
        Execute the supplied MOOSE application and return the YAML.
        """

        cache = os.path.join(MooseDocs.TEMP_DIR, 'moosedocs.yaml')
        exe = self.getConfig('executable')

        if os.path.exists(cache) and (os.path.getmtime(cache) >= os.path.getmtime(exe)):
            with open(cache, 'r') as fid:
                LOG.debug('Reading MooseYaml Pickle: ' + cache)
                return mooseutils.MooseYaml(pickle.load(fid))

        elif not (exe or os.path.exists(exe)):
            LOG.critical('The executable does not exist: %s', exe)
            raise Exception('Critical Error')

        else:
            LOG.debug("Executing %s to extract syntax.", exe)
            try:
                raw = mooseutils.runExe(exe, '--yaml')
                with open(cache, 'w') as fid:
                    LOG.debug('Writing MooseYaml Pickle: ' + cache)
                    pickle.dump(raw, fid)
                return mooseutils.MooseYaml(raw)
            except:
                LOG.critical('Failed to read YAML file, MOOSE and modules are likely not compiled'
                             ' correctly.')
                raise


    def extendMarkdown(self, md, md_globals):
        """
        Builds the extensions for MOOSE flavored markdown.
        """
        md.registerExtension(self)

        # Create a config object
        config = self.getConfigs()

        # Extract YAML
        exe_yaml = self.execute()

        # Generate YAML data from application
        # Populate the database for input file and children objects
        LOG.debug('Creating input file and source code use database.')
        database = common.MooseLinkDatabase(**config)

        # Populate the syntax
        self.syntax = collections.OrderedDict()
        for item in config['locations']:
            key = item.keys()[0]
            options = item.values()[0]
            options.setdefault('group', key)
            options.setdefault('name', key.replace('_', ' ').title())
            options.setdefault('install', config['install'])
            self.syntax[key] = common.MooseApplicationSyntax(exe_yaml, **options)

        # Inline Patterns
        params = MooseParameters(markdown_instance=md, syntax=self.syntax, **config)
        md.inlinePatterns.add('moose_parameters', params, '_begin')

        desc = MooseDescription(markdown_instance=md, syntax=self.syntax, **config)
        md.inlinePatterns.add('moose_description', desc, '_begin')

        object_markdown = MooseObjectSyntax(markdown_instance=md, syntax=self.syntax,
                                            database=database, **config)
        md.inlinePatterns.add('moose_object_syntax', object_markdown, '_begin')

        system_markdown = MooseActionSyntax(markdown_instance=md, syntax=self.syntax, **config)
        md.inlinePatterns.add('moose_system_syntax', system_markdown, '_begin')

        system_list = MooseActionList(markdown_instance=md, yaml=exe_yaml, syntax=self.syntax,
                                      **config)
        md.inlinePatterns.add('moose_system_list', system_list, '_begin')

def makeExtension(*args, **kwargs): #pylint: disable=invalid-name
    """
    Create the AppSyntaxExtension.
    """
    return AppSyntaxExtension(*args, **kwargs)

class MooseSyntaxBase(MooseMarkdownCommon, Pattern):
    """
    Base for MOOSE system/object pattern matching.

    Args:
      regex[str]: The regular expression to match.
      yaml[MooseYaml]: The MooseYaml object for the application.
      syntax[dict]: A dictionary of MooseApplicatinSyntax objects.
    """

    def __init__(self, regex, markdown_instance=None, syntax=None, **kwargs):
        MooseMarkdownCommon.__init__(self, **kwargs)
        Pattern.__init__(self, regex, markdown_instance)

        self._syntax = syntax

        # Error if the syntax was not supplied
        if not isinstance(self._syntax, dict):
            LOG.error("A dictionary of MooseApplicationSyntax objects must be supplied.")

    def getInfo(self, name):
        """
        Return the information object.
        """
        info = self.getObject(name)
        if info is None:
            return self.getAction(name)
        return info

    def getObject(self, name):
        """
        Return an MooseObject info object.
        """
        for syntax in self._syntax.itervalues():
            if syntax.hasObject(name):
                return syntax.getObject(name)
        return None

    def getAction(self, name):
        """
        Return an Action info object.
        """
        for syntax in self._syntax.itervalues():
            if syntax.hasAction(name):
                return syntax.getAction(name)
        return None

class MooseParameters(MooseSyntaxBase):
    """
    Creates parameter tables for Actions and MooseObjects.
    """

    RE = r'^!parameters\s+(.*?)(?:$|\s+)(.*)'

    @staticmethod
    def defaultSettings():
        """Default settings for MooseParameters."""
        settings = MooseSyntaxBase.defaultSettings()
        settings['title'] = ('Input Parameters', "Title to include prior to the parameter table.")
        settings['title_level'] = (2, "The HTML heading level to apply to the title")
        return settings

    def __init__(self, **kwargs):
        super(MooseParameters, self).__init__(self.RE, **kwargs)

    def handleMatch(self, match):
        """
        Return table(s) of input parameters.
        """

        # Extract Syntax and Settings
        syntax = match.group(2)
        settings = self.getSettings(match.group(3))

        # Locate description
        info = self.getInfo(syntax)
        if not info:
            return self.createErrorElement('Failed to locate MooseObject or Action for the '
                                           'command: !parameters {}'.format(syntax))

        # Create the tables (generate 'Required' and 'Optional' initially so that they come out in
        # the proper order)
        tables = collections.OrderedDict()
        tables['Required'] = MooseObjectParameterTable()
        tables['Optional'] = MooseObjectParameterTable()

        # Loop through the parameters in yaml object
        for param in info.parameters or []:
            name = param['group_name']
            if not name and param['required']:
                name = 'Required'
            elif not name and not param['required']:
                name = 'Optional'

            if name not in tables:
                tables[name] = MooseObjectParameterTable()
            tables[name].addParam(param)

        # Produces a debug message if parameters are empty, but generally we just want to include
        # the !parameters command, if parameters exist then a table is produce otherwise nothing
        # happens. This will allow for parameters to be added and the table appear if it was empty.
        if not any(tables.values()):
            LOG.debug('Unable to locate parameters for %s.', info.name)
        else:
            el = self.applyElementSettings(etree.Element('div'), settings)
            if settings['title']:
                title = etree.SubElement(el, 'h{}'.format(str(settings['title_level'])))
                title.text = settings['title']
            for key, table in tables.iteritems():
                if table:
                    subtitle = etree.SubElement(el, 'h{}'.format(str(settings['title_level'] + 1)))
                    subtitle.text = '{} {}'.format(key, 'Parameters')
                    el.append(table.html())
            return el

class MooseObjectSyntax(MooseSyntaxBase):
    """
    Extracts the description from a MooseObject parameters.

    Markdown Syntax:
    !<Keyword> <YAML Syntax> key=value, key1=value1, etc...

    Keywords Available:
      !inputfiles: Returns a set of lists containing links to the input files that use the syntax
      !childobjects: Returns a set of lists containing links to objects that inherit from this class
    """

    RE = r'^!(inputfiles|childobjects)\s+(.*?)(?:$|\s+)(.*)'

    def __init__(self, database=None, repo=None, **kwargs):
        super(MooseObjectSyntax, self).__init__(self.RE, **kwargs)

        # Input arguments
        self._input_files = database.inputs
        self._child_objects = database.children
        self._repo = repo

    def handleMatch(self, match):
        """
        Create a <p> tag containing the supplied description from the YAML dump.
        """

        # Extract match options and settings
        action = match.group(2)
        syntax = match.group(3)

        # Extract Settings
        settings = self.getSettings(match.group(4))

        # Locate description
        info = self.getObject(syntax)
        if not info:
            el = self.createErrorElement('Failed to locate MooseObject with syntax in command: '
                                         '!{} {}'.format(action, syntax), error=False)
        elif action == 'inputfiles':
            el = self.inputfilesElement(info, settings)
        elif action == 'childobjects':
            el = self.childobjectsElement(info, settings)
        return el

    def inputfilesElement(self, info, settings):
        """
        Return the links to input files and child objects.

        Args:
          node[dict]: YAML data node.
          styles[dict]: Styles from markdown.
        """
        # Print the item information
        el = self.applyElementSettings(etree.Element('div'), settings)
        el.set('id', '#input-files')
        el.set('class', 'section scrollspy')
        self._listhelper(info, 'Input Files', el, self._input_files)
        return el

    def childobjectsElement(self, info, settings):
        """
        Return the links to input files and child objects.

        Args:
          node[dict]: YAML data node.
          styles[dict]: Styles from markdown.
        """
        # Print the item information
        el = self.applyElementSettings(etree.Element('div'), settings)
        el.set('id', '#child-objects')
        el.set('class', 'section scrollspy')
        self._listhelper(info, 'Child Objects', el, self._child_objects)
        return el

    @staticmethod
    def _listhelper(info, title, parent, items):
        """
        Helper method for dumping link lists.

        Args:
          info: MooseObjectInfo object.
          title[str]: The level two header to apply to lists.
          parent[etree.Element]: The parent element the headers and lists are to be applied
          items[dict]: Dictionary of databases containing link information
        """
        has_items = False
        for k, db in items.iteritems():
            if info.name in db:
                has_items = True
                h3 = etree.SubElement(parent, 'h3')
                h3.text = k
                ul = etree.SubElement(parent, 'ul')
                ul.set('style', "max-height:350px;overflow-y:Scroll")
                for j in db[info.name]:
                    ul.append(j.html())

        if has_items:
            h2 = etree.Element('h2')
            h2.text = title
            parent.insert(0, h2)

class MooseDescription(MooseSyntaxBase):
    """
    Creates parameter tables for Actions and MooseObjects.
    """
    RE = r'^!description\s+(.*?)(?:$|\s+)(.*)'

    def __init__(self, **kwargs):
        super(MooseDescription, self).__init__(self.RE, **kwargs)

    def handleMatch(self, match):
        """
        Return the class description html element.
        """

        # Extract Syntax and Settings
        syntax = match.group(2)
        settings = self.getSettings(match.group(3))

        # Locate description
        info = self.getInfo(syntax)
        if not info:
            return self.createErrorElement('Failed to locate MooseObject or Action for the '
                                           'command: !description {}'.format(syntax))

        # Create an Error element, but do not produce warning/error LOG because the
        # moosedocs check/generate commands produce errors.
        if info.description is None:
            return self.createErrorElement('Failed to locate class description for {} '
                                           'syntax.'.format(info.name), error=False)

        # Create the html element with supplied styles
        el = self.applyElementSettings(etree.Element('p'), settings)
        el.text = info.description
        return el

class MooseActionList(MooseSyntaxBase):
    """
    Creates dynamic lists for Moose syntax.

    Examples:
    !systems
    !systems framework phase_field
    """

    RE = r'^!systems\s*(.*)'

    @staticmethod
    def defaultSettings():
        settings = MooseSyntaxBase.defaultSettings()
        settings['groups'] = (None, "The configured 'groups' to include.")
        settings['show_hidden'] = (False, "Show hidden syntax.")
        return settings

    def __init__(self, yaml=None, syntax=None, **kwargs):
        MooseSyntaxBase.__init__(self, self.RE, syntax=syntax, **kwargs)
        self._yaml = yaml

    def handleMatch(self, match):
        """
        Handle the regex match for this extension.
        """

        # Extract settings
        settings = self.getSettings(match.group(2))

        # Extract the data to consider
        groups = self._syntax.keys()
        if settings['groups']:
            groups = settings['groups'].split()

        # Build complete list of unique action objects
        actions = []
        keys = set()
        for syn in self._syntax.itervalues():
            for value in syn.actions().values():
                if value.key not in keys:
                    actions.append(value)
                    keys.add(value.key)

        # Create the primary element
        el = etree.Element('div')
        el.set('class', 'moose-system-list')

        # Alphabetize actions
        actions.sort(key=lambda action: action.name)

        # Storage structure for <div> tags to allow for nested item creation without
        # the need for complete actions tree or sorted action objects.
        folder_divs = dict()

        # Loop over keys
        for action in actions:

            # Do nothing if the syntax is hidden
            if (action.group in groups) and action.hidden and (not settings['show_hidden']):
                continue

            # Attempt to build the sub-objects table
            collection = MooseDocs.extensions.create_object_collection(action.key, \
                            self._syntax, groups=groups, show_hidden=settings['show_hidden'])

            # Do nothing if the table is empty or the supplied group is not desired
            if (not collection) and (action.group not in groups):
                continue

            # Loop through the syntax ("folders") and create the necessary html element
            folder = tuple(action.key.strip('/').split('/'))
            div = el
            for i in range(len(folder)):
                current = '/'.join(folder[0:i+1])

                 # If a <div> with the current name exists, use it, otherwise create the <div> and
                 # associated heading
                if current in folder_divs:
                    div = folder_divs[current]
                else:
                    div = etree.SubElement(div, 'div')
                    folder_divs[current] = div

                    h = etree.SubElement(div, 'h{}'.format(str(i+2)))
                    h.text = current
                    h_id = current.replace(' ', '_').lower()
                    h.set('id', h_id)

                    if i == 0:
                        div.set('class', 'section scrollspy')
                        div.set('id', h_id)

                    # Add link to action pages
                    a = etree.SubElement(h, 'a')
                    a.set('href', action.markdown)
                    i = etree.SubElement(a, 'i')
                    i.set('class', 'material-icons')
                    i.text = 'input'

                    # Create a chip showing where the action is defined
                    tag = etree.SubElement(h, 'div')
                    tag.set('class', 'chip moose-chip')
                    tag.text = action.group

            if collection:
                div.append(collection)

        return el

class MooseActionSyntax(MooseSyntaxBase):
    """
    Creates tables of sub-object/systems.

    !subobjects = A list of subobjects /* and /<type>
    !subsystems = A list of sub-syntax (e.g, Markers and Indicators)

    Available Settings:
      title[str]: Set the title of the section defined above the table.
    """

    RE = r'^!(subobjects|subsystems)\s+(.*?)(?:$|\s+)(.*)'

    @staticmethod
    def defaultSettings():
        settings = MooseSyntaxBase.defaultSettings()
        settings['title'] = ('default', "The title display prior to tables ('default' provides a "
                                        "title with the action name)")
        settings['title_level'] = (2, "The HTML heading level to apply to the title.")
        return settings

    def __init__(self, yaml=None, syntax=None, **kwargs):
        MooseSyntaxBase.__init__(self, self.RE, yaml=yaml, syntax=syntax, **kwargs)

    def handleMatch(self, match):
        """
        Handle the regex match.
        """
        # Extract match options and settings
        action = match.group(2)
        syntax = match.group(3)
        settings = self.getSettings(match.group(4))

        if settings['title'] == 'default':
            settings['title'] = 'Available {}-{}'.format(action[:3].title(), action[3:].title())

        if action == 'subobjects':
            el = self.subobjectsElement(syntax, settings)
        elif action == 'subsystems':
            el = self.subsystemsElement(syntax, settings)
        return el

    def subobjectsElement(self, sys_name, settings):
        """
        Create table of sub-objects.
        """
        collection = MooseDocs.extensions.create_object_collection(sys_name, self._syntax)
        if collection:
            el = self.applyElementSettings(etree.Element('div'), settings)
            if settings['title']:
                h2 = etree.SubElement(el, 'h2')
                h2.text = settings['title']
            el.append(collection)
        else:
            el = etree.Element('p')
        return el

    def subsystemsElement(self, sys_name, settings):
        """
        Create table of sub-systems.
        """
        collection = MooseDocs.extensions.create_system_collection(sys_name, self._syntax)
        if collection:
            el = self.applyElementSettings(etree.Element('div'), settings)
            if settings['title']:
                h2 = etree.SubElement(el, 'h{}'.format(settings['title_level']))
                h2.text = settings['title']
            el.append(collection)
        else:
            el = etree.Element('p')
        return el
