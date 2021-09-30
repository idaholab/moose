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
import MooseDocs
from ..base import components, HTMLRenderer
from ..tree import tokens, html, latex, pages
from ..common import exceptions

from . import command, core

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    return CivetExtension(**kwargs)

CivetTestBadges = tokens.newToken('CivetTestBadges', prefix=None, tests=list())
CivetTestReport = tokens.newToken('CivetTestReport', prefix=None, tests=list(), source=None)

class CivetExtension(command.CommandExtension):
    "Adds ability to include CIVET links."""

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['remotes'] = (dict(), "Remote CIVET repositories to pull result; each item in " \
                             "the dict should have another dict with a 'url', 'repo', and 'repo_url'.")
        config['branch'] = ('master', "The main stable branch for extracting test results.")
        config['download_test_results'] = (True, "Automatically download and aggregate test results for the current merge commits.")
        config['generate_test_reports'] = (True, "Generate test report pages, if results exist from download or local file(s).")
        config['test_reports_location'] = ('civet', "The local directory where the generated test reports will be inserted.")
        config['test_results_cache'] = (os.path.join(os.getenv('HOME'), '.local', 'share', 'civet', 'jobs'),
                                       "Default location for downloading CIVET results.")
        return config

    def __init__(self, *args, **kwargs):
        command.CommandExtension.__init__(self, *args, **kwargs)
        self.__database = dict()
        self.__hashes = None
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

    def hashes(self):
        """Return the git SHA-1 hashes for the CIVET results, returns None if not collected."""
        return self.__hashes

    def testBaseFileName(self, test):
        """
        Return the test page filename base.
        """
        return self.__test_result_numbers.get(test, None)

    def init(self):
        """(override) Generate test reports."""

        # Test result database
        self.__database = dict()
        self.__hashes = None

        # Only populate the database if the specified branch and author match current repository
        start = time.time()
        LOG.info("Collecting CIVET results...")
        branch = self.get('branch')
        remotes = mooseutils.git_remotes()
        for name, category in self.get('remotes').items():
            LOG.info("Gathering CIVET results for '%s'.", name)
            if category.get('download_test_results', self.get('download_test_results', True)):
                repo_url = category.get('repo_url', 'https://github.com').rstrip('/')
                repo_url += '/{}'.format(category.get('repo'))

                remote = remotes.get(repo_url, None)
                if remote is None:
                    remote = '__MooseDocs.extensions.civet__'
                    mooseutils.git_add_and_fetch_remote(repo_url, remote, branch)
                else:
                    mooseutils.get_fetch_remote(remote, branch)

                self.__hashes = mooseutils.get_civet_hashes(f'{remote}/{branch}')
                LOG.info("Downloading CIVET results for '%s' category.", name)

            local = mooseutils.eval_path(category.get('test_results_cache', self.get('test_results_cache')))
            site = (category['url'], category['repo'])
            local_db = mooseutils.get_civet_results(local=local,
                                                    hashes=self.__hashes,
                                                    site=site,
                                                    cache=local,
                                                    possible=['OK', 'FAIL', 'DIFF', 'TIMEOUT'],
                                                    logger=LOG)
            self.__database.update(local_db)
        LOG.info("Collecting CIVET results complete [%s sec.]", time.time() - start)

        if not self.__database and self.get('generate_test_reports', True):
            LOG.info("CIVET test result reports are being disabled, it requires results to exist and the specified branch ('%s') to match the current repository.", self.get('branch'))
            self.update(generate_test_reports=False)

        if self.get('generate_test_reports', True):
            self.__has_test_reports = True

            start = time.time()
            LOG.info("Creating CIVET result pages...")

            result_pages = list()
            report_root = self.get('test_reports_location')
            if not self.translator.findPage(report_root, exact=True, throw_on_zero=False):
                result_pages.append(pages.Directory(report_root, source=report_root))

            result_pages.append(pages.Source('{}/index.md'.format(report_root),
                                             source='{}/index.md'.format(report_root),
                                             read=False, tokenize=False))

            count = 0
            for key, item in self.__database.items():
                name = 'result_{}'.format(count)
                self.__test_result_numbers[key] = name
                count += 1

                fullname = '{}/{}.md'.format(report_root, name)
                result_pages.append(pages.Source(fullname, source=fullname,
                                                 read=False, tokenize=False, key=key))

            self.translator.addPages(result_pages)
            self.translator.executioner.initPages(result_pages)

            LOG.info("Creating CIVET result pages complete [%s sec.]", time.time() - start)

    def postTokenize(self, page, ast):
        """
        Add CIVET test report token.
        """
        key = page.get('key', None)
        if key is not None:
            h = core.Heading(ast, level=1)
            tokens.String(h, content='Test Results')
            core.Punctuation(h, content=':')
            core.LineBreak(h)
            core.Space(h)
            tokens.String(h, content=key)
            CivetTestReport(ast, tests=[key])

    def postRender(self, page, results):
        """
        Add CIVET links to test result pages.
        """
        report_root = self.get('test_reports_location')
        if page.source == '{}/index.md'.format(report_root):
            ol = html.Tag(results, 'ol')
            for key, item in self.__database.items():
                fullname = self.testBaseFileName(key) + '.html'
                html.Tag(html.Tag(ol, 'li'), 'a', href=fullname, string=key)

class CivetCommandBase(command.CommandComponent):
    COMMAND = 'civet'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['remote'] = (None, "The category to utilize for remote result lookup, see CivetExtension.")
        settings['url'] = (None, "Override for the repository url provided in the 'category' option, e.g. 'https://civet.inl.gov'.")
        settings['repo'] = (None, "Override for the repository name provided in the 'category' option, e.g. 'idaholab/moose'.")
        return settings

    def getCivetInfo(self, settings):
        available = self.extension.get('remotes')
        if len(available) > 0:
            category = available.get(settings.get('remote') or list(available.keys())[0])
            url = settings.get('url') or category['url']
            repo = settings.get('repo') or category['repo']
        else:
            url = settings.get('url')
            repo = settings.get('repo')
        return url, repo

class CivetMergeResultsCommand(CivetCommandBase):
    SUBCOMMAND = 'mergeresults'

    @staticmethod
    def defaultSettings():
        settings = CivetCommandBase.defaultSettings()
        return settings

    def createToken(self, parent, info, page, settings):
        site, repo = self.getCivetInfo(settings)

        rows = []
        for sha in self.extension.hashes() or list():
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

    def createToken(self, parent, info, page, settings):
        site, repo = self.getCivetInfo(settings)
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

    def createToken(self, parent, info, page, settings):
        return CivetTestBadges(parent, tests=settings.get('tests').split())

class CivetTestReportCommand(CivetCommandBase):
    SUBCOMMAND = 'report'

    @staticmethod
    def defaultSettings():
        config = CivetCommandBase.defaultSettings()
        config['tests'] = (None, "The name of the test(s) to report.")
        return config

    def createToken(self, parent, info, page, settings):
        return CivetTestReport(parent, tests=settings.get('tests').split())

class RenderCivetTestBadges(components.RenderComponent):

    def createLatex(self, parent, token, page):
        pass

    def createHTML(self, parent, token, page):
        pass

    def createMaterialize(self, parent, token, page):

        div = html.Tag(parent, 'div', class_='moose-civet-badges')
        prefix = token['prefix']
        for test in token['tests']:
            tname = '{}.{}'.format(prefix, test) if (prefix is not None) else test
            counts = collections.defaultdict(int)
            results = self.extension.results(tname)
            if results:
                for job, recipes in results.items():
                    for recipe in recipes:
                        counts[recipe.status] += 1

            base = self.extension.testBaseFileName(tname)
            if self.extension.hasTestReports() and (base is not None):
                report_root = self.extension.get('test_reports_location')
                fname = os.path.join(self.translator.get("destination"), report_root, base + '.html')
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

        prefix = token['prefix']
        for key in token['tests']:
            tname = '{}.{}'.format(prefix, key) if (prefix is not None) else key
            results = self.extension.results(tname)

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
