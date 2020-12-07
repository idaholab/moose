#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import re
import copy
import codecs
import logging
import collections
import traceback
import moosetree
import uuid
import json
import time
import pyhit

import MooseDocs
import mooseutils
import moosesqa

from .. import common
from ..common import exceptions
from ..base import components, MarkdownReader, LatexRenderer, HTMLRenderer
from ..tree import tokens, html, latex, pages
from . import core, command, floats, autolink, civet, appsyntax, table

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    return SQAExtension(**kwargs)

SQARequirementMatrix = tokens.newToken('SQARequirementMatrix')
SQARequirementMatrixItem = tokens.newToken('SQARequirementMatrixItem', label=None, reqname=None,
                                           satisfied=True)
SQARequirementMatrixListItem = tokens.newToken('SQARequirementMatrixListItem', label=None)
SQARequirementText = tokens.newToken('SQARequirementText')
SQARequirementDesign = tokens.newToken('SQARequirementDesign', design=[], line=None, filename=None)
SQARequirementIssues = tokens.newToken('SQARequirementIssues', issues=[], line=None, filename=None, url=None)
SQARequirementCollections = tokens.newToken('SQARequirementCollections', collections=[])
SQARequirementSpecification = tokens.newToken('SQARequirementSpecification',
                                              spec_name=None, spec_path=None)
SQARequirementPrequisites = tokens.newToken('SQARequirementPrequisites', specs=[])
SQARequirementDetails = tokens.newToken('SQARequirementDetails')
SQARequirementDetailItem = tokens.newToken('SQARequirementDetailItem', label=None)

SQARequirementMatrixHeading = tokens.newToken('SQARequirementMatrixHeading', category=None)

# use 'Token' suffix to avoid confusion with moosesqa.SQAReport object
SQADocumentReportToken = tokens.newToken('SQADocumentReportToken', reports=None)
SQAMooseAppReportToken = tokens.newToken('SQAMooseAppReportToken', reports=None)
SQARequirementReportToken = tokens.newToken('SQARequirementReportToken', reports=None)

LATEX_REQUIREMENT = """
\\DeclareDocumentEnvironment{Requirement}{m}{%
  \\begin{minipage}[t]{0.08\\textwidth}%
    #1
  \\end{minipage}
  \\hfill
  \\begin{minipage}[t]{0.9\\textwidth}%
}{%
  \\end{minipage}
}
"""

class SQAExtension(command.CommandExtension):

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['url'] = ('https://github.com', "Deprecated, see 'repos'.")
        config['repo'] = (None, "Deprecated, see 'repos'.")

        config['repos'] = (dict(default="https://github.com/idaholab/moose"),
                           "The repository locations for linking issues, set 'default' to allow " \
                           "'#1234' or add additional keys to allow for foo#1234.")
        config['categories'] = (dict(), "A dictionary of category names that includes a " \
                                        "dictionary with 'directories' and optionally 'specs' " \
                                        ", 'dependencies', 'repo', 'reports'.")
        config['requirement-groups'] = (dict(), "Allows requirement group names to be changed")
        config['reports'] = (None, "Build SQA reports for dashboard creation.")
        config['add_run_exception_to_failure_analysis'] = (True, "Automatically include RunException tests in the 'FAILURE_ANALYSIS' collection.")

        # Disable by default to allow for updates to applications
        config['active'] = (False, config['active'][1])
        return config

    def __init__(self, *args, **kwargs):
        command.CommandExtension.__init__(self, *args, **kwargs)

        # Always include MOOSE and libMesh
        self['repos'].update(dict(moose="https://github.com/idaholab/moose",
                                  libmesh="https://github.com/libMesh/libmesh"))

        # Build requirements sets
        self.__has_civet = False
        self.__requirements = dict()
        self.__dependencies = dict()
        self.__remotes = dict()
        self.__counts = collections.defaultdict(int)
        self.__reports = dict()

        # Deprecate 'url' and 'repo' config options
        url = self.get('url')
        repo = self.get('repo')
        if repo is not None:
            msg = "The 'url' and 'repo' config options for MooseDocs.extensions.sqa are deprecated,"\
                 " add the 'repos' option with a 'default' entry instead."
            LOG.warning(msg)
            self['repos'].update(dict(default="{}/{}".format(url, repo)))

    def preExecute(self):
        """Initialize requirement information"""

        # Clear any existing counts
        self.__counts.clear()

        # Do not repopulate
        if self.__requirements:
            return

        start = time.time()
        LOG.info("Gathering SQA requirement information...")

        repos = self.get('repos')
        for index, (category, info) in enumerate(self.get('categories').items(), 1):
            specs = info.get('specs', ['tests'])
            repo = info.get('repo', 'default')
            reports = info.get('reports', None)
            directories = []

            for d in info.get('directories'):
                path = mooseutils.eval_path(d)
                if not os.path.isdir(path):
                    path = os.path.join(MooseDocs.ROOT_DIR, d)
                if not os.path.isdir(path):
                    msg = "Input directory does not exist: %s"
                    LOG.error(msg, path)

                directories.append(path)

            # Create requirement database
            self.__requirements[category] = moosesqa.get_requirements(directories, specs, 'F', index)

            # Create dependency database
            self.__dependencies[category] = info.get('dependencies', [])

            # Create remote repository database
            self.__remotes[category] = repos.get(repo, None)

            # Create reports from included SQA apps (e.g., moose/modulus/stochastic_tools)
            if reports:
                self.__reports[category] = moosesqa.get_sqa_reports(reports)

        # Report for the entire app (e.g, moose/modules/doc)
        general_reports = self.get('reports')
        if general_reports is not None:
            self.__reports['__empty__'] = moosesqa.get_sqa_reports(general_reports)

        # The following attempts to save computation time by avoiding re-computing the application
        # syntax for each report. This is done by extracting the syntax from the appsyntax extension
        # and using it within the SQAMooseAppReport objects.

        # Get the syntax tree and executable info from the appsyntax extension
        app_syntax = None
        exe_dir = None
        exe_name = None
        for ext in self.translator.extensions:
            if isinstance(ext, appsyntax.AppSyntaxExtension):
                app_syntax = ext.syntax
                exe_dir, exe_name = os.path.split(ext.executable) if ext.executable else (None, None)
                break

        # Setup the SQAMooseAppReports to use the syntax from the running app
        for reports in self.__reports.values():
            for app_report in reports[2]:
                app_report.app_syntax = app_syntax
                app_report.exe_directory = exe_dir
                app_report.exe_name = exe_name.split('-')[0] if exe_name else None
                if app_syntax is None:
                    msg = 'Attempting to inject application syntax into SQAMooseAppReport, but the syntax does not exist.'
                    LOG.warning(msg)

        # Add RunException tests to FAILURE_ANALYSIS collection
        for req_category in self.__requirements.values():
            for requirements in req_category.values():
                for req in requirements:
                    t_types = req.types
                    if (req.collections is None) and (t_types is not None) and ('RunException' in t_types):
                        req.collections = set(['FAILURE_ANALYSIS'])

        LOG.info("Gathering SQA requirement information complete [%s sec.]", time.time() - start)

    def hasCivetExtension(self):
        """Return True if the CivetExtension exists."""
        return self.__has_civet

    def requirements(self, category):
        """Return the requirements dictionaries."""
        req = self.__requirements.get(category, None)
        if req is None:
            raise exceptions.MooseDocsException("Unknown or missing 'category': {}", category)
        return req

    def dependencies(self, category):
        """Return the dependency list for given category."""
        dep = self.__dependencies.get(category, None)
        if dep is None:
            raise exceptions.MooseDocsException("Unknown or missing 'category': {}", category)
        return dep

    def remote(self, category):
        """Return the remote URL for the given category."""
        rem = self.__remotes.get(category, None)
        if rem is None:
            raise exceptions.MooseDocsException("Unknown or missing 'category': {}", category)
        return rem

    def reports(self, category):
        """Return the SQAReport objects"""
        rep = self.__reports.get(category, None)
        if rep is None:
            raise exceptions.MooseDocsException("Unknown or missing 'category': {}", category)
        return rep

    def increment(self, key):
        """Increment and return count for requirements matrix."""
        self.__counts[key] += 1
        return self.__counts[key]

    def extend(self, reader, renderer):
        self.requires(core, command, autolink, floats, table)

        for ext in self.translator.extensions:
            if ext.name == 'civet':
                self.__has_civet = True

        self.addCommand(reader, SQARequirementsCommand())
        self.addCommand(reader, SQARequirementsMatrixCommand())
        self.addCommand(reader, SQAVerificationCommand())
        self.addCommand(reader, SQACrossReferenceCommand())
        self.addCommand(reader, SQACollectionsCommand())
        self.addCommand(reader, SQACollectionsListCommand())
        self.addCommand(reader, SQADependenciesCommand())
        self.addCommand(reader, SQADocumentCommand())
        self.addCommand(reader, SQAReportCommand())

        renderer.add('SQARequirementMatrix', RenderSQARequirementMatrix())
        renderer.add('SQARequirementMatrixItem', RenderSQARequirementMatrixItem())
        renderer.add('SQARequirementMatrixListItem', RenderSQARequirementMatrixListItem())
        renderer.add('SQARequirementMatrixHeading', RenderSQARequirementMatrixHeading())
        renderer.add('SQARequirementText', RenderSQARequirementText())
        renderer.add('SQARequirementDesign', RenderSQARequirementDesign())
        renderer.add('SQARequirementIssues', RenderSQARequirementIssues())
        renderer.add('SQARequirementCollections', RenderSQARequirementCollections())
        renderer.add('SQARequirementSpecification', RenderSQARequirementSpecification())
        renderer.add('SQARequirementPrequisites', RenderSQARequirementPrequisites())
        renderer.add('SQARequirementDetails', RenderSQARequirementDetails())
        renderer.add('SQARequirementDetailItem', RenderSQARequirementDetailItem())
        renderer.add('SQADocumentReportToken', RenderSQADocumentReport())
        renderer.add('SQARequirementReportToken', RenderSQARequirementReport())
        renderer.add('SQAMooseAppReportToken', RenderSQAMooseAppReport())

        if isinstance(renderer, LatexRenderer):
            renderer.addPackage('xcolor')
            renderer.addPreamble(LATEX_REQUIREMENT)

        if isinstance(renderer, HTMLRenderer):
            renderer.addCSS('sqa_moose', "css/sqa_moose.css")
            renderer.addJavaScript('sqa_moose', "js/sqa_moose.js")


class SQARequirementsCommand(command.CommandComponent):
    COMMAND = 'sqa'
    SUBCOMMAND = 'requirements'

    @staticmethod
    def defaultSettings():
        config = command.CommandComponent.defaultSettings()
        config['category'] = (None, "Provide the category.")
        config['link'] = (True, "Enable/disable the linking of test specifications and " \
                                 "test files.")
        config['link-spec'] = (True, "Enable/disable the link of the test specification only, " \
                               "the 'link' setting must be true.")
        config['link-design'] = (True, "Enable/disable the link of the test design only, " \
                                 "the 'link' setting must be true.")
        config['link-issues'] = (True, "Enable/disable the link of the test issues only, " \
                                 "the 'link' setting must be true.")
        config['link-prerequisites'] = (True, "Enable/disable the link of the test prerequisites, "\
                                        "the 'link' setting must be true.")
        config['link-results'] = (True, "Enable/disable the link to the test results.")
        config['link-collections'] = (True, "Enable/disable the collections badge(s).")

        return config

    def createToken(self, parent, info, page):
        category = self.settings.get('category', None)
        if category == '_empty_':
            return parent

        group_map = self.extension.get('group_map', dict())
        for group, requirements in self.extension.requirements(category).items():
            group = group_map.get(group, group.replace('_', ' ').title())

            matrix = SQARequirementMatrix(parent)
            SQARequirementMatrixHeading(matrix, category=category, string=str(group))
            for req in requirements:
                self._addRequirement(matrix, info, page, req, requirements, category)

        return parent

    def _addRequirement(self, parent, info, page, req, requirements, category):
        # Do nothing if deprecated
        if req.deprecated:
            return

        reqname = "{}:{}".format(req.path, req.name) if req.path != '.' else req.name
        item = SQARequirementMatrixItem(parent, label=req.label, reqname=reqname,
                                        satisfied=req.testable)
        text = SQARequirementText(item)
        if req.requirement is not None:
            self.reader.tokenize(text, req.requirement, page, MarkdownReader.INLINE, info.line, report=False)
            for token in moosetree.iterate(item):
                if token.name == 'ErrorToken':
                    msg = common.report_error("Failed to tokenize SQA requirement.",
                                          req.filename,
                                          req.requirement_line,
                                          req.requirement,
                                          token['traceback'],
                                          'SQA TOKENIZE ERROR')
                    LOG.critical(msg)

        if req.details:
            details = SQARequirementDetails(item)
            for detail in req.details:
                ditem = SQARequirementDetailItem(details)
                text = SQARequirementText(ditem)
                self.reader.tokenize(text, detail.detail, page, MarkdownReader.INLINE, info.line, \
                                     report=False)

        if self.settings['link']:
            if self.settings['link-spec']:
                p = SQARequirementSpecification(item, spec_path=req.path, spec_name=req.name)

                hit_root = pyhit.load(req.filename)
                h = moosetree.find(hit_root, lambda n: n.name==req.name)
                content = h.render()

                floats.create_modal_link(p,
                                         title=reqname, string=reqname,
                                         content=core.Code(None, language='text', content=content))

            if self.settings['link-design'] and req.design:
                SQARequirementDesign(item, filename=req.filename, design=req.design,
                                     line=req.design_line)

            if self.settings['link-issues'] and req.issues:
                SQARequirementIssues(item, filename=req.filename, issues=req.issues,
                                     line=req.issues_line, url=self.extension.remote(category))

            if self.settings['link-collections'] and req.collections:
                SQARequirementCollections(item, collections=req.collections)

            if self.settings.get('link-prerequisites', False) and req.prerequisites:
                labels = []
                for other in requirements:
                    if (other is not req) and (other.path == req.path) and (req.prerequisites.intersection(other.names)):
                        labels.append((other.path, other.name, other.label))
                if labels:
                    SQARequirementPrequisites(item, specs=labels)

            if self.settings.get('link-results', False):
                keys = list()

                for detail in req.details:
                    keys.append('{}.{}/{}'.format(req.path, req.name, detail.name))

                if not keys:
                    keys.append('{}.{}'.format(req.path, req.name))

                if self.extension.hasCivetExtension():
                    civet.CivetTestBadges(item, tests=keys)

class SQACrossReferenceCommand(SQARequirementsCommand):
    COMMAND = 'sqa'
    SUBCOMMAND = 'cross-reference'

    @staticmethod
    def defaultSettings():
        config = SQARequirementsCommand.defaultSettings()
        config['category'] = (None, "Provide the category.")
        config.pop('link-prerequisites')
        return config

    def createToken(self, parent, info, page):
        design = collections.defaultdict(list)
        category = self.settings.get('category')
        if category == '_empty_':
            return parent

        for requirements in self.extension.requirements(category).values():
            for req in requirements:
                if req.design is None:
                    continue
                for d in req.design:
                    try:
                        node = self.translator.findPage(d)
                        design[node].append(req)
                    except exceptions.MooseDocsException:
                        msg = "Failed to locate the design page '{}'".format(d)
                        LOG.critical(common.report_error(msg,
                                                         req.filename,
                                                         req.design_line,
                                                         ' '.join(req.design),
                                                         traceback.format_exc(),
                                                         'SQA ERROR'))

        for node, requirements in design.items():
            matrix = SQARequirementMatrix(parent)
            heading = SQARequirementMatrixHeading(matrix, category=category)
            autolink.AutoLink(heading, page=str(node.local))
            for req in requirements:
                self._addRequirement(matrix, info, page, req, requirements, category)

        return parent

class SQACollectionsCommand(SQARequirementsCommand):
    COMMAND = 'sqa'
    SUBCOMMAND = ('collections', 'types')

    @staticmethod
    def defaultSettings():
        config = SQARequirementsCommand.defaultSettings()
        config['category'] = (None, "Provide the category.")
        config['items'] = (None, "List the 'collections' or 'types', if not provided all will be shown.")
        config.pop('link-prerequisites')
        return config

    def createToken(self, parent, info, page):
        category = self.settings.get('category')
        if category == '_empty_':
            return parent

        collection_map = collections.defaultdict(list)
        allowed = self.settings.get('items')
        if allowed is not None:
            allowed = set(allowed.strip().split())

        for requirements in self.extension.requirements(category).values():
            for req in requirements:
                attrib = getattr(req, info['subcommand'])
                if attrib is None:
                    continue
                for c in attrib:
                    if (allowed is None) or (c in allowed):
                        collection_map[c].append(req)

        for item, requirements in collection_map.items():
            matrix = SQARequirementMatrix(parent)
            heading = SQARequirementMatrixHeading(matrix, category=category, string=item)
            for req in requirements:
                self._addRequirement(matrix, info, page, req, requirements, category)

        return parent

class SQACollectionsListCommand(command.CommandComponent):
    COMMAND = 'sqa'
    SUBCOMMAND = 'collections-list'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings.update(floats.caption_settings())
        settings['prefix'] = ('Table', settings['prefix'][1])
        return settings

    def createToken(self, parent, info, page):
        flt = floats.create_float(parent, self.extension, self.reader, page, self.settings,
                                  token_type=table.TableFloat, **self.attributes)

        rows = [[k,v] for k, v in moosesqa.MOOSESQA_COLLECTIONS.items()]
        tbl = table.builder(rows, headings=['Collection', 'Description'])
        tbl.parent = flt
        return parent

class SQARequirementsMatrixCommand(command.CommandComponent):
    COMMAND = 'sqa'
    SUBCOMMAND = 'requirements-matrix'
    COUNT = 1

    @staticmethod
    def defaultSettings():
        config = command.CommandComponent.defaultSettings()
        config['category'] = (None, "Provide the category.")
        config['prefix'] = (None, "The letter prefix (e.g., 'P' for performance) for the type of "
                                  "requirement.")
        config['heading'] = (None, "Requirement matrix heading.")
        return config

    def createToken(self, parent, info, page):
        content = info['block'] if 'block' in info else info['inline']

        if self.settings['prefix'] is None:
            msg = "The 'prefix' option is required."
            raise exceptions.MooseDocsException(msg)

        # Extract the unordered list
        self.reader.tokenize(parent, content, page, MarkdownReader.BLOCK, info.line)
        ul = parent.children[-1]
        ul.parent = None

        # Check the list type
        if ul.name != 'UnorderedList':
            msg = "The content is required to be an unordered list (i.e., use '-')."
            raise exceptions.MooseDocsException(msg)

        # Build the matrix
        prefix = self.settings['prefix']
        label = '{}{:d}'.format(prefix, self.extension.increment(prefix))
        matrix = SQARequirementMatrix(parent)

        heading = self.settings['heading']
        if heading is not None:
            head = SQARequirementMatrixHeading(matrix)
            self.reader.tokenize(head, heading, page, MarkdownReader.INLINE, info.line)

        for i, item in enumerate(ul.children):
            matrix_item = SQARequirementMatrixListItem(matrix, label='{}.{:d}'.format(label, i))
            for child in item:
                child.parent = matrix_item

        return parent

class SQAVerificationCommand(command.CommandComponent):
    COMMAND = 'sqa'
    SUBCOMMAND = ('verification', 'validation')

    @staticmethod
    def defaultSettings():
        config = command.CommandComponent.defaultSettings()
        config['category'] = (None, "Provide the category.")
        return config

    def createToken(self, parent, info, page):
        category = self.settings.get('category')
        if category == '_empty_':
            return parent

        matrix = SQARequirementMatrix(parent)
        for requirements in self.extension.requirements(category).values():
            for req in requirements:
                if getattr(req, info['subcommand']) is not None:
                    self._addRequirement(matrix, info, page, req)

        return parent

    def _addRequirement(self, parent, info, page, req):
        reqname = "{}:{}".format(req.path, req.name) if req.path != '.' else req.name
        item = SQARequirementMatrixItem(parent, label=req.label, reqname=reqname)
        self.reader.tokenize(item, req.requirement, page, MarkdownReader.INLINE, info.line, report=False)
        for token in moosetree.iterate(item):
            if token.name == 'ErrorToken':
                msg = common.report_error("Failed to tokenize SQA requirement.",
                                          req.filename,
                                          req.requirement_line,
                                          req.requirement,
                                          token['traceback'],
                                          'SQA TOKENIZE ERROR')
                LOG.critical(msg)

        p = core.Paragraph(item)
        tokens.String(p, content='Specification: ')

        content = common.read(req.filename)
        floats.create_modal_link(p,
                                 string=reqname,
                                 content=core.Code(None, language='text', content=content),
                                 title=str(req.filename))

        p = core.Paragraph(item)
        tokens.String(p, content='Documentation: ')
        for filename in  getattr(req, info['subcommand']):
            autolink.AutoLink(p, page=str(filename))
            core.Space(p)

class SQADependenciesCommand(command.CommandComponent):
    COMMAND = 'sqa'
    SUBCOMMAND = 'dependencies'

    @staticmethod
    def defaultSettings():
        config = command.CommandComponent.defaultSettings()
        config['suffix'] = (None, "Provide the filename suffix to include.")
        config['category'] = (None, "Provide the category.")
        return config

    def createToken(self, parent, info, page):
        suffix = self.settings['suffix']
        category = self.settings['category']
        if category == '_empty_':
            depends = self.extension.get('categories').keys()
        else:
            depends = self.extension.dependencies(category) or \
                self.extension.get('categories').keys()

        ul = core.UnorderedList(parent)
        for dep in depends:
            if dep != category:
                fname = '{}_{}.md'.format(dep, suffix)
                autolink.AutoLink(core.ListItem(ul), page='sqa/{}'.format(fname),
                                  optional=True, warning=True, class_='moose-sqa-dependency')
        return parent

class SQADocumentCommand(command.CommandComponent):
    COMMAND = 'sqa'
    SUBCOMMAND = 'document'

    @staticmethod
    def defaultSettings():
        config = command.CommandComponent.defaultSettings()
        config['suffix'] = (None, "Provide the filename suffix to include.")
        config['category'] = (None, "Provide the category.")
        return config

    def createToken(self, parent, info, page):
        category = self.settings.get('category')
        suffix = self.settings.get('suffix')
        return autolink.AutoLink(parent, page='sqa/{}_{}.md'.format(category, suffix),
                                 optional=True, warning=True)

class SQAReportCommand(command.CommandComponent):
    COMMAND = 'sqa'
    SUBCOMMAND = 'report'

    @staticmethod
    def defaultSettings():
        config = command.CommandComponent.defaultSettings()
        config['config_file'] = (None, "Provide the config YAML file for gathering reports to display on the dashboard.")
        config['category'] = (None, "Provide the category.")
        return config

    def createToken(self, parent, info, page):
        category = self.settings.get('category') or  '__empty__'
        doc_reports, req_reports, app_reports = self.extension.reports(category)
        core.Heading(parent, string='Software Quality Status Report(s)', level=2)
        SQADocumentReportToken(parent, reports=doc_reports)
        SQARequirementReportToken(parent, reports=req_reports)
        SQAMooseAppReportToken(parent, reports=app_reports)
        return parent

class RenderSQARequirementMatrix(core.RenderUnorderedList):

    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'ul', class_='moose-sqa-requirements')

    def createMaterialize(self, parent, token, page):
        tag = self.createHTML(parent, token, page)
        tag.addClass('collection')
        tag.addClass('with-header')
        return tag

    def createLatex(self, parent, token, page):
        return parent

class RenderSQARequirementMatrixItem(core.RenderListItem):
    def createHTML(self, parent, token, page):
        attributes = copy.copy(token.attributes)

        li = html.Tag(parent, 'li', **attributes)
        num = html.Tag(li, 'span', string=token['label'], class_='moose-sqa-requirement-number')

        if not token['satisfied']:
            num.addClass('moose-sqa-requirement-unsatisfied')

        return html.Tag(li, 'span', class_='moose-sqa-requirement-content')

    def createMaterialize(self, parent, token, page):
        attributes = copy.copy(token.attributes)
        id_ = token['reqname']

        li = html.Tag(parent, 'li', class_="collection-item", **attributes)
        num = html.Tag(li, 'span', string=token['label'], id_=id_,
                       class_='moose-sqa-requirement-number')
        num.addClass('tooltipped')
        num['data-tooltip'] = id_

        if not token['satisfied']:
            num.addClass('moose-sqa-requirement-unsatisfied')

        return html.Tag(li, 'span', class_='moose-sqa-requirement-content')

    def createLatex(self, parent, token, page):
        label = token['label']
        if not token['satisfied']:
            brace = latex.Brace()
            latex.Command(brace, 'textcolor',
                          args=[latex.Brace(string='red')],
                          string=label)
            args = [brace]
        else:
            args = [latex.Brace(string=label)]
        return latex.Environment(parent, 'Requirement', args=args)

class RenderSQARequirementMatrixListItem(RenderSQARequirementMatrixItem):

    def createHTML(self, parent, token, page):
        li = html.Tag(parent, 'li', token)
        html.Tag(li, 'span', string=token['label'], class_='moose-sqa-requirement-number')
        return html.Tag(li, 'span', class_='moose-sqa-requirement-content')

    def createMaterialize(self, parent, token, page):
        li = html.Tag(parent, 'li', token, class_="collection-item")
        html.Tag(li, 'span', string=token['label'], class_='moose-sqa-requirement-number')
        return html.Tag(li, 'span', class_='moose-sqa-requirement-content')

    def createLatex(self, parent, token, page):
        args = [latex.Brace(string=token['label'])]
        return latex.Environment(parent, 'Requirement', args=args)

class RenderSQARequirementMatrixHeading(core.RenderListItem):

    def createHTML(self, parent, token, page):
        return None

    def createMaterialize(self, parent, token, page):
        tag = html.Tag(parent, 'li', class_='collection-header')
        category = token.get('category')
        if category:
            tag.insert(0, html.String(content='{}: '.format(category)))
        return tag

    def createLatex(self, parent, token, page):
        prefix = token.siblings[0]['label'].split('.')[0]
        return latex.Command(parent, 'section*', string=prefix + ':~', escape=False)

class RenderSQARequirementText(components.RenderComponent):
    def createHTML(self, parent, token, page):
        return parent

    def createLatex(self, parent, token, page):
        return parent

class RenderSQARequirementDesign(autolink.RenderLinkBase):

    def findDesign(self, filename, design, line):
        node = self.translator.findPage(design, throw_on_zero=False)
        if node is None:
            msg = "Unable to locate the design page: {}\n    {}:{}"
            raise exceptions.MooseDocsException(msg, design, filename, line)
        return node

    def createHTML(self, parent, token, page):
        p = html.Tag(parent, 'p', string='Design: ', class_='moose-sqa-items')
        for design in token['design']:
            node = self.findDesign(token['filename'], design, token['line'])
            if node is not None:
                link = autolink.AutoLink(None, page=page)
                link.info = token.info
                self.createHTMLHelper(p, link, page, node)
            else:
                html.Tag(p, 'a', string=str(design), class_='moose-error')

    def createLatex(self, parent, token, page):
        prev = token.previous
        if prev and prev.name != 'RenderSQARequirementDetails':
            latex.Command(parent, 'newline', start='\n', end='\n')

        latex.String(parent, content='Design:~', escape=False)
        no_seperator = True
        for design in token['design']:
            if not no_seperator:
                latex.String(parent, content='; ')
                no_seperator = False

            node = self.findDesign(token['filename'], design, token['line'])
            if node:
                link = autolink.AutoLink(None, page=page)
                link.info = token.info
                self.createLatexHelper(parent, link, page, node)
            else:
                latex.Command(parent, 'textcolor', args=[latex.Brace(string='red')], string=design)

class RenderSQARequirementIssues(components.RenderComponent):

    ISSUE_RE = re.compile(r"(?P<key>\w+)?#(?P<issues>[0-9]+)")
    COMMIT_RE = re.compile(r"(?:(?P<key>\w+):)?(?P<commit>[0-9a-f]{10,40})")

    def __urlHelper(self, regex, name, issue, token):
        """Function for creating issue/commit links."""
        repos = self.extension['repos']
        default = token['url']
        match = regex.search(issue)
        if match:
            key = match.group('key')
            repo = repos.get(key, None)
            if (key is not None) and (repo is None):
                msg = "Unknown key '{}' for MooseDocs.extensions.sqa 'repos' config.\n    {}:{}"
                raise exceptions.MooseDocsException(msg, key, token['filename'], token['line'])
            repo = repo or default
            url = "{}/{}/{}".format(repo, name, match.group(name))
            return url

    def getURL(self, issue, token):
        url = None
        url = self.__urlHelper(self.ISSUE_RE, 'issues', issue, token)
        if url is None:
            url = self.__urlHelper(self.COMMIT_RE, 'commit', issue, token)

        if (url is None) and (issue != ''):
            msg = "Unknown issue number or commit (commit SHA-1 must be at least 10 digits): "\
                  "{}\n    {}:{}".format(issue, token['filename'], token['line'])
            LOG.error(msg)

        return url

    def createHTML(self, parent, token, page):

        p = html.Tag(parent, 'p', string='Issue(s): ', class_='moose-sqa-items')
        for issue in token['issues']:

            url = self.getURL(issue, token)
            if url is None:
                html.Tag(p, 'a', string=issue, class_='moose-error')
            else:
                html.Tag(p, 'a', href=url, string=str(issue))

    def createLatex(self, parent, token, page):
        prev = token.previous
        if prev and prev.name != 'SQARequirementDetails':
            latex.Command(parent, 'newline', start='\n', end='\n')
        latex.String(parent, content='Issue(s):~', escape=False)
        no_seperator = True

        for issue in token['issues']:
            if not no_seperator:
                latex.String(parent, content='; ')
                no_seperator = False

            url = self.getURL(issue, token)
            if url is None:
                latex.Command(parent, 'textcolor', args=[latex.Brace(string='red')], string=issue)
            else:
                latex.Command(parent, 'href', args=[latex.Brace(string=url)], string=str(issue))

class RenderSQARequirementCollections(components.RenderComponent):
    def createHTML(self, parent, token, page):
        p = html.Tag(parent, 'p', string='Collections: ', class_='moose-sqa-items')
        for item in token['collections']:
            html.Tag(p, 'span', string=str(item))

    def createLatex(self, parent, token, page):
        prev = token.previous
        if prev and prev.name != 'SQARequirementDetails':
            latex.Command(parent, 'newline', start='\n', end='\n')
        latex.String(parent, content='Collections:~', escape=False)
        no_seperator = True

        for item in token['collections']:
            if not no_seperator:
                latex.String(parent, content='; ')
                no_seperator = False
            latex.Command(parent, 'textcolor', args=[latex.Brace(string='blue')], string=item)

class RenderSQARequirementSpecification(components.RenderComponent):
    def createMaterialize(self, parent, token, page):
        return html.Tag(parent, 'p', string='Specification: ')

    def createHTML(self, parent, token, page):
        spath = token['spec_path']
        if spath == '.':
            spec = 'Specification: {}'.format(token['spec_name'])
        else:
            spec = 'Specification: {}:{}'.format(token['spec_path'], token['spec_name'])
        html.Tag(parent, 'p', string=spec)

    def createLatex(self, parent, token, page):
        prev = token.previous
        if prev and prev.name != 'SQARequirementDetails':
            latex.Command(parent, 'newline', start='\n', end='\n')
        spath = token['spec_path']
        if spath == '.':
            spec = 'Specification: {}'.format(token['spec_name'])
        else:
            spec = 'Specification: {}:{}'.format(token['spec_path'], token['spec_name'])
        latex.String(parent, content=spec)

class RenderSQARequirementPrequisites(components.RenderComponent):
    def createHTML(self, parent, token, page):
        p = html.Tag(parent, 'p', string='Prerequisite(s): ', class_='moose-sqa-items')

        for path, name, label in token['specs']:
            url = '#{}:{}'.format(path, name)
            html.Tag(p, 'a', href=url, string=label)

    def createLatex(self, parent, token, page):
        prev = token.previous
        if prev and prev.name != 'SQARequirementDetails':
            latex.Command(parent, 'newline', start='\n', end='\n')
        latex.String(parent, content='Prerequisite(s):~', escape=False)

        labels = [label for _, _, label in token['specs']]
        latex.String(parent, content='; '.join(labels))

class RenderSQARequirementDetails(components.RenderComponent):
    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'ol', class_='moose-sqa-details-list')

    def createLatex(self, parent, token, page):
        return latex.Environment(parent, 'enumerate', after_begin='', after_end='', info=token.info)

class RenderSQARequirementDetailItem(components.RenderComponent):
    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'li', class_='moose-sqa-detail-item')

    def createLatex(self, parent, token, page):
        latex.Command(parent, 'item', start='\n', end=' ')
        return parent

class RenderSQAReport(components.RenderComponent):

    def createHTML(self, parent, token, page):
        msg = "The '!sqa report' command in not supported with plain HTML output, it is being ignored."
        LOG.warning(msg)

    def createLatex(self, parent, token, page):
        msg = "The '!sqa report' command in not supported with Latex output, it is being ignored."
        LOG.warning(msg)

    def createMaterialize(self, parent, token, page):
        reports = token['reports']
        if not reports:
            return

        ul = html.Tag(parent, 'ul', class_='collapsible')
        for report in reports:
            report.color_text = False
            report.getReport()

            # Header
            li = html.Tag(ul, 'li')
            hdr = html.Tag(li, 'div', class_='collapsible-header', string=report.title)
            if report.status == report.Status.WARNING:
                badge = ('WARNING', 'yellow')
            elif report.status == report.Status.ERROR:
                badge = ('ERROR', 'red')
            else:
                badge = ('OK', 'green')
            html.Tag(hdr, 'span', string=self._getBadgeText(report.status),
                     class_='badge {}'.format(self._getBadgeColor(report.status)))

            # Body
            body = html.Tag(li, 'div', class_='collapsible-body')
            collection = html.Tag(body, 'ul', class_='collection')
            for key, mode in report.logger.modes.items():
                cnt = report.logger.counts[key]
                item = html.Tag(collection, 'li', class_='collection-item')
                span = html.Tag(item, 'span', string=key)
                color = self._getBadgeColor(mode) if (cnt > 0) else 'green'
                badge = html.Tag(item, 'span', class_='badge {}'.format(color), string=str(cnt))
                if cnt > 0:
                    self._addMessageModal(item, color, report.logger.text(key))

    @staticmethod
    def _addMessageModal(item, color, text):
        unique_id = uuid.uuid4()

        trigger = html.Tag(item, 'a', class_='modal-trigger', href='#{}'.format(unique_id),
                           style="float:right;margin-left:10px;margin-right:10px;")
        i = html.Tag(trigger, 'i', string='help', class_='material-icons', style='color:{};'.format(color))

        modal = html.Tag(item, 'div', id_=str(unique_id), class_="modal modal-fixed-footer")
        content = html.Tag(modal, class_='modal-content')
        for t in text:
            html.Tag(content, 'p', string='<br>'.join(t.split('\n')))
        footer = html.Tag(modal, 'div', class_='modal-footer')
        html.Tag(footer, 'a', class_='modal-close btn-flat', string='Close')


    @staticmethod
    def _getBadgeColor(status):
        if status == moosesqa.SQAReport.Status.WARNING or status == logging.WARNING:
            return 'orange'
        elif status == moosesqa.SQAReport.Status.ERROR or status == logging.ERROR:
            return 'red'
        return 'green'

    @staticmethod
    def _getBadgeText(status):
        if status == moosesqa.SQAReport.Status.WARNING:
            return 'WARNING'
        elif status == moosesqa.SQAReport.Status.ERROR:
            return 'ERROR'
        return 'OK'

class RenderSQADocumentReport(RenderSQAReport):
    def createMaterialize(self, parent, token, page):
        html.Tag(parent, 'h3', string='Necessary SQA Document Report(s)')
        super().createMaterialize(parent, token, page)

class RenderSQARequirementReport(RenderSQAReport):
    def createMaterialize(self, parent, token, page):
        html.Tag(parent, 'h3', string='Requirement Completion Report(s)')
        super().createMaterialize(parent, token, page)

class RenderSQAMooseAppReport(RenderSQAReport):
    def createMaterialize(self, parent, token, page):
        html.Tag(parent, 'h3', string='Application Design Page Report(s)')
        super().createMaterialize(parent, token, page)
