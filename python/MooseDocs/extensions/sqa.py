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
import itertools
import pyhit

import MooseDocs
import mooseutils
import moosesqa

from .. import common
from ..common import exceptions
from ..base import components, MarkdownReader, LatexRenderer, HTMLRenderer
from ..tree import tokens, html, latex, pages
from . import core, command, floats, autolink, civet, appsyntax, table, modal

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    return SQAExtension(**kwargs)

SQARequirementMatrix = tokens.newToken('SQARequirementMatrix')
SQARequirementMatrixHeading = tokens.newToken('SQARequirementMatrixHeading', category=None)
SQARequirementMatrixItem = tokens.newToken('SQARequirementMatrixItem', label=None, reqname=None, satisfied=True)
SQARequirementMatrixListItem = tokens.newToken('SQARequirementMatrixListItem', label=None)
SQARequirementText = tokens.newToken('SQARequirementText')
SQARequirementDesign = tokens.newToken('SQARequirementDesign', design=[], line=None, filename=None)
SQARequirementIssues = tokens.newToken('SQARequirementIssues', issues=[], line=None, filename=None, url=None)
SQARequirementCollections = tokens.newToken('SQARequirementCollections', collections=[])
SQARequirementTypes = tokens.newToken('SQARequirementTypes', types=[])
SQARequirementPrerequisites = tokens.newToken('SQARequirementPrerequisites', specs=[])
SQARequirementDetails = tokens.newToken('SQARequirementDetails')
SQARequirementDetailItem = tokens.newToken('SQARequirementDetailItem', label=None)

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
        config['include_non_testable_requirements'] = (False, "Control display of skipped requirements.")
        config['requirement-groups'] = (dict(), "Allows requirement group names to be changed")
        config['reports'] = (None, "Build SQA reports for dashboard creation.")
        config['default_collection'] = ('FUNCTIONAL', "The default requirement collection.")
        config['append_run_exception_to_failure_analysis'] = (True, "Automatically include RunException tests in the 'FAILURE_ANALYSIS' collection.")

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

        for category, info in self.get('categories').items():
            specs = info.get('specs', ['tests'])
            repo = info.get('repo', 'default')
            reports = info.get('reports', None)

            # Build the requirements, dependencies, remotes, and reports data structures
            directories = list()
            for d in info.get('directories', []):
                path = mooseutils.eval_path(d)
                if not os.path.isdir(path):
                    path = os.path.join(MooseDocs.ROOT_DIR, d)
                if not os.path.isdir(path):
                    msg = "Input directory does not exist: %s"
                    LOG.error(msg, path)
                    continue
                directories.append(path)

            # Create requirement database from tests
            self.__requirements[category] = moosesqa.get_requirements_from_tests(directories, specs,
                                                                                 self.get('include_non_testable_requirements'))

            # Create dependency database
            self.__dependencies[category] = info.get('dependencies', [])

            # Create remote repository database
            repos = self.get('repos')
            self.__remotes[category] = repos.get(repo, None)

            # Create reports from included SQA apps (e.g., moose/modulus/stochastic_tools)
            if reports:
                self.__reports[category] = moosesqa.get_sqa_reports(reports)

        # Number the requirements
        for c, req_dict in enumerate(self.__requirements.values(), start=1):
            moosesqa.number_requirements(req_dict, c)

        # Report for the entire app (e.g, moose/modules/doc)
        general_reports = self.get('reports')
        if general_reports is not None:
            self.__reports['_empty_'] = moosesqa.get_sqa_reports(general_reports)

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
        for app_report in itertools.chain.from_iterable(reports[2] for reports in self.__reports.values() if reports[2] is not None):
            app_report.app_syntax = app_syntax
            app_report.exe_directory = exe_dir
            app_report.exe_name = exe_name.rsplit('-', maxsplit=1)[0] if exe_name else None
            if app_syntax is None:
                msg = 'Attempting to inject application syntax into SQAMooseAppReport, but the syntax does not exist.'
                LOG.warning(msg)

        # Set default collection and add RunException tests to FAILURE_ANALYSIS collection
        d_type = self['default_collection']
        for req_category in self.__requirements.values():
            for requirements in req_category.values():
                for req in requirements:
                    t_types = req.types
                    if (req.collections is None) and (t_types is not None) and ('RunException' in t_types):
                        req.collections = set([d_type, 'FAILURE_ANALYSIS'])
                    elif (req.collections is None):
                        req.collections = set([d_type])

        LOG.info("Gathering SQA requirement information complete [%s sec.]", time.time() - start)

    def postTokenize(self, page, ast):
        """Remove empty SQARequirementMatrix tokens"""
        for node in moosetree.findall(ast.root, func=lambda n: n.name == 'SQARequirementMatrix'):
            if not any(n.name == 'SQARequirementMatrixItem' for n in node.children):
                node.parent = None

    def hasCivetExtension(self):
        """Return True if the CivetExtension exists."""
        return self.__has_civet

    def requirements(self, category):
        """Return the requirements dictionaries."""
        r = self.__requirements.get(category, None)
        if r is None:
            raise exceptions.MooseDocsException("Unknown or missing 'category': {}", category)
        return r

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
        self.requires(core, command, autolink, floats, table, modal)

        for ext in self.translator.extensions:
            if ext.name == 'civet':
                self.__has_civet = True

        self.addCommand(reader, SQARequirementsCommand())
        self.addCommand(reader, SQAVerificationCommand())
        self.addCommand(reader, SQACrossReferenceCommand())
        self.addCommand(reader, SQADependenciesCommand())
        self.addCommand(reader, SQADocumentCommand())
        self.addCommand(reader, SQAReportCommand())
        self.addCommand(reader, SQARecordCommand())

        renderer.add('SQARequirementMatrix', RenderSQARequirementMatrix())
        renderer.add('SQARequirementMatrixItem', RenderSQARequirementMatrixItem())
        renderer.add('SQARequirementMatrixListItem', RenderSQARequirementMatrixListItem())
        renderer.add('SQARequirementMatrixHeading', RenderSQARequirementMatrixHeading())
        renderer.add('SQARequirementText', RenderSQARequirementText())
        renderer.add('SQARequirementDesign', RenderSQARequirementDesign())
        renderer.add('SQARequirementIssues', RenderSQARequirementIssues())
        renderer.add('SQARequirementCollections', RenderSQARequirementCollections())
        renderer.add('SQARequirementTypes', RenderSQARequirementTypes())
        renderer.add('SQARequirementPrerequisites', RenderSQARequirementPrerequisites())
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
        config['category'] = (None, "Provide the category, as listed in the extension 'categories' configuration.")
        config['collections'] = (None, "Limit the Requirement list to the specified collections (e.g., 'FUNCTIONAL FAILURE_ANALYSIS').")
        config['types'] = (None, "Limit the Requirement list to the specified test types (e.g., 'RunException Exodiff').")

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
        config['link-types'] = (True, "Enable/disable the types badge(s).")
        config['link-verification'] = (True, "Enable/disable the verification file link.")
        config['link-validation'] = (True, "Enable/disable the validation file link.")
        return config

    def createToken(self, parent, info, page, settings):
        category = settings['category']
        collections = settings['collections']
        if category == '_empty_':
            return parent

        group_map = self.extension.get('requirement-groups', dict())

        # Used in check for number of requirements in a given collection (functional, usability, etc.)
        # while working through a given category (kernels, etc.)
        num_collection_reqs = 0

        for group, requirements in self.extension.requirements(category).items():
            group = group_map.get(group, group.replace('_', ' ').title())
            matrix = SQARequirementMatrix(parent)
            SQARequirementMatrixHeading(matrix, category=category, string=str(group))
            for req in requirements:
                self._addRequirement(matrix, info, page, req, requirements, category, settings)
                if (collections is not None) and (req.collections is not None) and any(c in collections for c in req.collections):
                    num_collection_reqs += 1

        if num_collection_reqs == 0:
            tokens.String(parent, content="No requirements of this type exist for this application, beyond those of its dependencies.")

        return parent

    def _addRequirement(self, parent, info, page, req, requirements, category, settings):
        collections = settings['collections']
        types = settings['types']

        # Skip add if ...
        if req.deprecated:
            return
        if (collections is not None) and (req.collections is not None) and not any(c in collections for c in req.collections):
            return
        if (types is not None) and not any(t in types for t in req.types or set()):
            return

        item = SQARequirementMatrixItem(parent, label=req.label, reqname=req.name, satisfied=req.testable)
        text = SQARequirementText(item)
        if req.requirement is not None:
            self.reader.tokenize(text, req.requirement, page, MarkdownReader.INLINE, info.line, report=False)
            for t in moosetree.iterate(item):
                if t.name == 'ErrorToken':
                    msg = common.report_error("Failed to tokenize SQA requirement.",
                                              req.filename,
                                              req.requirement_line,
                                              req.requirement,
                                              token.get('traceback', None),
                                              'SQA TOKENIZE ERROR')
                    LOG.critical(msg)

        if req.details:
            details = SQARequirementDetails(item)
            for detail in req.details:
                ditem = SQARequirementDetailItem(details)
                text = SQARequirementText(ditem)
                self.reader.tokenize(text, detail.detail, page, MarkdownReader.INLINE,
                                                info.line, report=False)
                for t in moosetree.iterate(ditem):
                    if t.name == 'ErrorToken':
                        msg = common.report_error("Failed to tokenize SQA requirement detail.",
                                                  detail.filename,
                                                  detail.detail_line,
                                                  detail.detail,
                                                  token.get('traceback', None),
                                                  'SQA TOKENIZE ERROR')
                        LOG.critical(msg)

        if settings.get('link', False):
            if settings.get('link-spec', False):
                p = core.Paragraph(item)
                tokens.String(p, content='Specification(s): ')
                for spec in req.specifications:
                    if p.count > 2:
                        tokens.String(p, content=', ')
                    s = modal.ModalLink(p, string=spec.name, content=core.Code(None, content=spec.text))

            if settings.get('link-design', False) and req.design:
                SQARequirementDesign(item, filename=req.filename, design=req.design,
                                     line=req.design_line)

            if settings.get('link-issues', False) and req.issues:
                SQARequirementIssues(item, filename=req.filename, issues=req.issues,
                                     line=req.issues_line, url=self.extension.remote(category))

            if settings.get('link-collections', False) and req.collections:
                SQARequirementCollections(item, collections=req.collections)

            if settings.get('link-types', False) and req.types:
                SQARequirementTypes(item, types=req.types)

            if settings.get('link-prerequisites', False) and req.prerequisites:
                labels = []
                for other in requirements:
                    if (other is not req) and (req.prerequisites.intersection(other.names)):
                        labels.append((other.name, other.label))
                if labels:
                    SQARequirementPrerequisites(item, specs=labels)

            if settings.get('link-results', False) and self.extension.hasCivetExtension():
                civet.CivetTestBadges(item, prefix=req.prefix, tests=req.names)

            if settings.get('link-verification', False) and req.verification:
                p = core.Paragraph(item)
                tokens.String(p, content='Verification: ')
                for filename in req.verification:
                    autolink.AutoLink(p, page=str(filename))
                    core.Space(p)

            if settings.get('link-validation', False) and req.validation:
                p = core.Paragraph(item)
                tokens.String(p, content='Validation: ')
                for filename in req.validation:
                    autolink.AutoLink(p, page=str(filename))
                    core.Space(p)

class SQACrossReferenceCommand(SQARequirementsCommand):
    SUBCOMMAND = 'cross-reference'

    @staticmethod
    def defaultSettings():
        config = SQARequirementsCommand.defaultSettings()
        config.pop('link-prerequisites')
        return config

    def createToken(self, parent, info, page, settings):
        category = settings['category']
        if category == '_empty_':
            return parent

        design = collections.defaultdict(list)
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
                self._addRequirement(matrix, info, page, req, requirements, category, settings)

        return parent

class SQAVerificationCommand(SQARequirementsCommand):
    SUBCOMMAND = ('verification', 'validation')

    @staticmethod
    def defaultSettings():
        config = SQARequirementsCommand.defaultSettings()
        config.pop('link-prerequisites')
        config.pop('link-verification')
        config.pop('link-validation')
        return config

    def createToken(self, parent, info, page, settings):
        category = settings['category']
        if category == '_empty_':
            return parent

        subcommand = info['subcommand']
        settings['link-{}'.format(subcommand)] = True
        matrix = SQARequirementMatrix(parent)
        SQARequirementMatrixHeading(matrix, category=category, string=subcommand.title())
        for requirements in self.extension.requirements(category).values():
            for req in requirements:
                if getattr(req, subcommand) is not None:
                    self._addRequirement(matrix, info, page, req, requirements, category, settings)

        return parent

class SQADependenciesCommand(command.CommandComponent):
    COMMAND = 'sqa'
    SUBCOMMAND = 'dependencies'

    @staticmethod
    def defaultSettings():
        config = command.CommandComponent.defaultSettings()
        config['suffix'] = (None, "Provide the filename suffix to include.")
        config['category'] = (None, "Provide the category.")
        return config

    def createToken(self, parent, info, page, settings):
        suffix = settings['suffix']
        category = settings['category'] or None
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

    def createToken(self, parent, info, page, settings):
        category = settings['category']
        if category == '_empty_':
            return parent

        suffix = settings.get('suffix')
        return autolink.AutoLink(parent, page='sqa/{}_{}.md'.format(category, suffix),
                                 optional=True, warning=True)

class SQAReportCommand(command.CommandComponent):
    COMMAND = 'sqa'
    SUBCOMMAND = 'report'

    @staticmethod
    def defaultSettings():
        config = command.CommandComponent.defaultSettings()
        config['category'] = (None, "Provide the category.")
        return config

    def createToken(self, parent, info, page, settings):
        category = settings.get('category') or '_empty_'
        doc_reports, req_reports, app_reports = self.extension.reports(category)
        core.Heading(parent, string='Software Quality Status Report(s)', level=2)
        SQADocumentReportToken(parent, reports=doc_reports)
        SQARequirementReportToken(parent, reports=req_reports)
        SQAMooseAppReportToken(parent, reports=app_reports)
        return parent


    def _createVandVMatrix(self, token, page, subcommand):
        root = tokens.Token(None)
        return root

    def _addRequirement(self, parent, token, page, req, requirements, category):
        item = SQARequirementMatrixItem(parent, label=req.label, reqname=req.name, satisfied=req.testable)
        text = SQARequirementText(item)
        if req.requirement is not None:
            self.translator.reader.tokenize(text, req.requirement, page, MarkdownReader.INLINE, token.info.line, report=False)
            for t in moosetree.iterate(item):
                if t.name == 'ErrorToken':
                    msg = common.report_error("Failed to tokenize SQA requirement.",
                                              req.filename,
                                              req.requirement_line,
                                              req.requirement,
                                              token.get('traceback', None),
                                              'SQA TOKENIZE ERROR')
                    LOG.critical(msg)

        if req.details:
            details = SQARequirementDetails(item)
            for detail in req.details:
                ditem = SQARequirementDetailItem(details)
                text = SQARequirementText(ditem)
                self.translator.reader.tokenize(text, detail.detail, page, MarkdownReader.INLINE,
                                                token.info.line, report=False)
                for t in moosetree.iterate(ditem):
                    if t.name == 'ErrorToken':
                        msg = common.report_error("Failed to tokenize SQA requirement detail.",
                                                  detail.filename,
                                                  detail.detail_line,
                                                  detail.detail,
                                                  token.get('traceback', None),
                                                  'SQA TOKENIZE ERROR')
                        LOG.critical(msg)

        if token['link']:
            if token['link_spec']:
                p = core.Paragraph(item)
                tokens.String(p, content='Specification(s): ')
                for spec in req.specifications:
                    p = SQARequirementSpecification(item, spec_name=req.name)
                    s = modal.ModalSourceLink(p, string=spec.name, content=core.Code(None, content=content))

            if token['link_design'] and req.design:
                SQARequirementDesign(item, filename=req.filename, design=req.design,
                                     line=req.design_line)

            if token['link_issues'] and req.issues:
                SQARequirementIssues(item, filename=req.filename, issues=req.issues,
                                     line=req.issues_line, url=self.extension.remote(category))

            if token['link_collections'] and req.collections:
                SQARequirementCollections(item, collections=req.collections)

            if token['link_prereq'] and req.prerequisites:
                labels = []
                for other in requirements:
                    if (other is not req) and (req.prerequisites.intersection(other.names)):
                        labels.append((other.name, other.label))
                if labels:
                    SQARequirementPrerequisites(item, specs=labels)

            if token['link_results'] and self.extension.hasCivetExtension():
                civet.CivetTestBadges(item, prefix=req.prefix, tests=req.names)

            if token['link_verification'] and req.verification:
                p = core.Paragraph(item)
                tokens.String(p, content='Verification: ')
                for filename in req.verification:
                    autolink.AutoLink(p, page=str(filename))
                    core.Space(p)

            if token['link_validation'] and req.validation:
                p = core.Paragraph(item)
                tokens.String(p, content='Validation: ')
                for filename in req.validation:
                    autolink.AutoLink(p, page=str(filename))
                    core.Space(p)

class SQARecordCommand(command.CommandComponent):
    COMMAND = 'sqa'
    SUBCOMMAND = 'records'

    @staticmethod
    def defaultSettings():
        config = command.CommandComponent.defaultSettings()
        config['category'] = (None, "Provide the category.")
        return config

    def createToken(self, parent, info, page, settings):
        category = settings.get('category') or '_empty_'
        doc_reports, _, _ = self.extension.reports(category)
        if doc_reports is not None:
            core.Heading(parent, string='Software Quality Records', level=2)
            ul = core.UnorderedList(parent)
            for report in doc_reports:
                for document in report.documents:
                    li = core.ListItem(ul)
                    if document.filename is not None:
                        split_name = document.filename.split('#', maxsplit=1)
                        fname = split_name[0]
                        bookmark = split_name[1] if len(split_name) > 1 else None
                        autolink.AutoLink(li, page=fname, bookmark=bookmark, string=document.title)
                    else:
                        tokens.String(li, content=document.title)
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

    ISSUE_RE = re.compile(r"(?P<key>.+)?#(?P<issues>[0-9]+)")
    COMMIT_RE = re.compile(r"(?:(?P<key>.+):)?(?P<commit>[0-9a-f]{10,40})")

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
        p = html.Tag(parent, 'p', string='Collection(s): ', class_='moose-sqa-items')
        for item in token['collections']:
            html.Tag(p, 'span', string=str(item))

    def createLatex(self, parent, token, page):
        prev = token.previous
        if prev and prev.name != 'SQARequirementDetails':
            latex.Command(parent, 'newline', start='\n', end='\n')
        latex.String(parent, content='Collection(s):~', escape=False)
        no_seperator = True

        for item in token['collections']:
            if not no_seperator:
                latex.String(parent, content='; ')
                no_seperator = False
            latex.Command(parent, 'textcolor', args=[latex.Brace(string='blue')], string=item)

class RenderSQARequirementTypes(components.RenderComponent):
    def createHTML(self, parent, token, page):
        p = html.Tag(parent, 'p', string='Type(s): ', class_='moose-sqa-items')
        for item in token['types']:
            html.Tag(p, 'span', string=str(item))

    def createLatex(self, parent, token, page):
        prev = token.previous
        if prev and prev.name != 'SQARequirementDetails':
            latex.Command(parent, 'newline', start='\n', end='\n')
        latex.String(parent, content='Type(s):~', escape=False)
        no_seperator = True

        for item in token['types']:
            if not no_seperator:
                latex.String(parent, content='; ')
                no_seperator = False
            latex.Command(parent, 'textcolor', args=[latex.Brace(string='blue')], string=item)

class RenderSQARequirementPrerequisites(components.RenderComponent):
    def createHTML(self, parent, token, page):
        p = html.Tag(parent, 'p', string='Prerequisite(s): ', class_='moose-sqa-items')

        for label in token['specs']:
            url = '#{}'.format(label[0])
            html.Tag(p, 'a', href=url, string=label[1])

    def createLatex(self, parent, token, page):
        prev = token.previous
        if prev and prev.name != 'SQARequirementDetails':
            latex.Command(parent, 'newline', start='\n', end='\n')
        latex.String(parent, content='Prerequisite(s):~', escape=False)

        labels = [label[1] for label in token['specs']]
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
        if token['reports']:
            html.Tag(parent, 'h3', string='Necessary SQA Document Report(s)')
            super().createMaterialize(parent, token, page)

class RenderSQARequirementReport(RenderSQAReport):
    def createMaterialize(self, parent, token, page):
        if token['reports']:
            html.Tag(parent, 'h3', string='Requirement Completion Report(s)')
            super().createMaterialize(parent, token, page)

class RenderSQAMooseAppReport(RenderSQAReport):
    def createMaterialize(self, parent, token, page):
        if token['reports']:
            html.Tag(parent, 'h3', string='Application Design Page Report(s)')
            super().createMaterialize(parent, token, page)
