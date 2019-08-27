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
import logging
import time
import mooseutils
import collections
import uuid
from ..base import components, HTMLRenderer
from ..tree import tokens, html, latex, pages
from ..common import exceptions

from . import command, core

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    return CivetExtension(**kwargs)

CivetTestBadges = tokens.newToken('CivetTestBadges', tests=list())
CivetTestReport = tokens.newToken('CivetTestReport', tests=list(), source=None)

class CivetExtension(command.CommandExtension):
    "Adds ability to include CIVET links."""

    CIVET_ROOT = 'civet'

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['categories'] = (dict(), "Available repositories")
        config['gather_test_results'] = (True, "Automatically assemble test results.")
        config['generate_test_reports'] = (True, "Generate test report pages.")
        return config

    def __init__(self, *args, **kwargs):
        command.CommandExtension.__init__(self, *args, **kwargs)
        self.__database = dict()
        self.__test_result_numbers = dict()
        self.__has_test_reports = False

    def hasTestReports(self):
        """Returns True if the test report pages were generated."""
        return self.__has_test_reports

    def extend(self, reader, renderer):
        self.requires(command)

        self.addCommand(reader, CivetResultsCommand())
        self.addCommand(reader, CivetMergeResultsCommand())
        self.addCommand(reader, CivetTestBadgesCommand())
        self.addCommand(reader, CivetTestReportCommand())

        renderer.add('CivetTestBadges', RenderCivetTestBadges())
        renderer.add('CivetTestReport', RenderCivetTestReport())

        if isinstance(renderer, HTMLRenderer):
            renderer.addCSS('civet_moose', "css/civet_moose.css")

    def results(self, name):
        """Return the test results for the supplied name."""
        return self.__database.get(name, None)

    def testBaseFileName(self, test):
        """
        Return the test page filename base.
        """
        return self.__test_result_numbers.get(test, None)

    def init(self):
        """(override) Generate test reports."""

        if self.get('gather_test_results', True):
            # Test result database
            start = time.time()
            LOG.info("Gathering CIVET results...")

            sites = list()
            for category in self.get('categories').values():
                sites.append((category['url'], category['repo']))

            hashes = mooseutils.git_merge_commits()
            self.__database = mooseutils.get_civet_results(hashes,
                                                           possible=['OK', 'FAIL', 'DIFF', 'TIMEOUT'],
                                                           sites=sites,
                                                           logger=LOG)

            LOG.info("Gathering CIVET results complete [%s sec.]", time.time() - start)

        if self.get('generate_test_reports', True):
            if not self.get('gather_test_results', True):
                LOG.error("'generate_test_reports' requires 'gather_test_results' to operate.")
                return

            self.__has_test_reports = True
            start = time.time()
            LOG.info("Creating CIVET result pages...")

            self.translator.addContent(pages.Directory(self.CIVET_ROOT, source=self.CIVET_ROOT))
            self.translator.addContent(pages.Source('{}/index.md'.format(self.CIVET_ROOT),
                                                    source='{}/index.md'.format(self.CIVET_ROOT),
                                                    read=False, tokenize=False))

            count = 0
            for key, item in self.__database.items():
                name = 'result_{}'.format(count)
                self.__test_result_numbers[key] = name
                count += 1

                fullname = '{}/{}.md'.format(self.CIVET_ROOT, name)
                self.translator.addContent(pages.Source(fullname, source=fullname, read=False,
                                                        tokenize=False,
                                                        key=key))

            LOG.info("Creating CIVET result pages complete [%s sec.]", time.time() - start)

    def postTokenize(self, ast, page, meta, reader):
        """
        Add CIVET test report token.
        """
        key = page.get('key', None)
        if key is not None:
            CivetTestReport(ast, tests=[key])

    def postRender(self, results, page, meta, renderer):
        """
        Add CIVET links to test result pages.
        """
        if page.source == '{}/index.md'.format(self.CIVET_ROOT):
            ol = html.Tag(results, 'ol')
            for key, item in self.__database.items():
                fullname = self.testBaseFileName(key) + '.html'
                html.Tag(html.Tag(ol, 'li'), 'a', href=fullname, string=key)

class CivetCommandBase(command.CommandComponent):
    COMMAND = 'civet'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['category'] = (None, "The category to utilize, see CivetExtension.")
        settings['url'] = (None, "Override for the repository url provided in the 'category' option, e.g. 'https://civet.inl.gov'.")
        settings['repo'] = (None, "Override for the repository name provided in the 'category' option, e.g. 'idaholab/moose'.")
        return settings

    def getCivetInfo(self):
        available = self.extension.get('categories')
        if len(available) > 0:
            category = available.get(self.settings.get('category') or list(available.keys())[0])
            url = self.settings.get('url') or category['url']
            repo = self.settings.get('repo') or category['repo']
        else:
            url = self.settings.get('url')
            repo = self.settings.get('repo')
        return url, repo

class CivetMergeResultsCommand(CivetCommandBase):
    SUBCOMMAND = 'mergeresults'

    @staticmethod
    def defaultSettings():
        settings = CivetCommandBase.defaultSettings()
        return settings

    def createToken(self, parent, info, page):
        site, repo = self.getCivetInfo()

        rows = []
        for sha in mooseutils.git_merge_commits():
            url = '{}/sha_events/{}/{}'.format(site, repo, sha)
            link = core.Link(parent, url=url, string=sha)
            core.LineBreak(parent)
        return parent

class CivetResultsCommand(CivetCommandBase):
    SUBCOMMAND = 'results'

    @staticmethod
    def defaultSettings():
        settings = CivetCommandBase.defaultSettings()
        return settings

    def createToken(self, parent, info, page):
        site, repo = self.getCivetInfo()
        sha = mooseutils.git_commit()
        url = '{}/sha_events/{}/{}'.format(site, repo, sha)
        if info['inline']:
            return core.Link(parent, url=url)
        else:
            return core.Link(parent, string=sha, url=url)

class CivetTestBadgesCommand(CivetCommandBase):
    SUBCOMMAND = 'badges'

    @staticmethod
    def defaultSettings():
        config = CivetCommandBase.defaultSettings()
        config['tests'] = (None, "The name of the test(s) to report.")
        return config

    def createToken(self, parent, info, page):
        return CivetTestBadges(parent, tests=self.settings.get('tests').split())

class CivetTestReportCommand(CivetCommandBase):
    SUBCOMMAND = 'report'

    @staticmethod
    def defaultSettings():
        config = CivetCommandBase.defaultSettings()
        config['tests'] = (None, "The name of the test(s) to report.")
        return config

    def createToken(self, parent, info, page):
        return CivetTestReport(parent, tests=self.settings.get('tests').split())

class RenderCivetTestBadges(components.RenderComponent):

    def createLatex(self, parent, token, page):
        pass

    def createHTML(self, parent, token, page):
        pass

    def createMaterialize(self, parent, token, page):

        div = html.Tag(parent, 'div', class_='moose-civet-badges')
        for test in token['tests']:
            counts = collections.defaultdict(int)
            results = self.extension.results(test)
            if results:
                for job, recipes in results.items():
                    for recipe in recipes:
                        counts[recipe.status] += 1

            base = self.extension.testBaseFileName(test)
            if self.extension.hasTestReports() and (base is not None):
                fname = os.path.join(self.translator.get("destination"), CivetExtension.CIVET_ROOT, base + '.html')
                location = os.path.relpath(fname, os.path.dirname(page.destination))
                a = html.Tag(div, 'a', href=location)
            else:
                a = html.Tag(div, 'span')

            for key, count in counts.items():
                badge = html.Tag(a, 'span', class_="new badge", string=str(count))
                badge['data-badge-caption'] = key
                badge['data-status'] = key.lower()

            if 'OK' not in counts:
                parent.parent.addClass('moose-civet-fail')

class RenderCivetTestReport(components.RenderComponent):

    def createLatex(self, parent, token, page):
        pass

    def createHTML(self, parent, token, page):
        pass

    def createMaterialize(self, parent, token, page):

        for key in token['tests']:
            results = self.extension.results(key)

            html.Tag(parent, 'h1', string='Test Results')
            html.Tag(parent, 'p', string=key)

            div = html.Tag(parent, 'div', class_='moose-civet-test-report')

            tbl = html.Tag(div, 'table')
            tr = html.Tag(tbl, 'tr')
            html.Tag(tr, 'th', string='Status')
            html.Tag(tr, 'th', string='Job')
            html.Tag(tr, 'th', string='Recipe')

            for job, tests in results.items():
                for item in tests:
                    tr = html.Tag(tbl, 'tr')
                    td = html.Tag(tr, 'td', string=item.status)
                    td['data-status'] = item.status.lower()
                    tr_job = html.Tag(tr, 'td')
                    html.Tag(tr, 'td', string=item.recipe)

                    link = html.Tag(tr_job, 'span')
                    html.Tag(link, 'a', href='{}/job/{}'.format(item.url, job), string=str(job))
