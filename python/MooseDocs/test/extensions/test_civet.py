#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html
import os
import unittest
import logging
from mooseutils import civet_results
from MooseDocs.test import MooseDocsTestCase
from MooseDocs.extensions import core, command, civet
from MooseDocs.tree import pages
from MooseDocs import base

logging.basicConfig()


class CivetTestCase(MooseDocsTestCase):
    def assertURL(self, node):
        url = node["url"]
        self.assertIn("https://civet.inl.gov/sha_events/idaholab/moose/", url)
        sha = url[48:]
        self.assertEqual(len(sha), 40)


class TestInlineCivet(CivetTestCase):
    """Test the the locally supplied url and repo are working. The code for this is shared so it
    should be good enough to test it with one command."""

    EXTENSIONS = [core, command, civet]
    RESULTS = "[!civet!results url=https://civet.inl.gov repo=idaholab/moose](Results)"
    RESULTS2 = "[!civet!results url=https://civet.inl.gov repo=idaholab/moose]"

    def setupExtension(self, ext):
        if ext == civet:
            return dict(
                generate_test_reports=False, test_results_cache="/tmp/civet/jobs"
            )

    def testResults(self):
        ast = self.tokenize(self.RESULTS)
        self.assertSize(ast, 1)
        self.assertToken(ast(0), "Paragraph", size=1)
        self.assertToken(ast(0, 0), "Link", size=1)
        self.assertToken(ast(0, 0, 0), "Word", size=0, content="Results")
        self.assertURL(ast(0, 0))

    def testResults2(self):
        ast = self.tokenize(self.RESULTS2)
        self.assertSize(ast, 1)
        self.assertToken(ast(0), "Paragraph", size=1)
        self.assertToken(ast(0, 0), "Link", size=1)
        self.assertToken(ast(0, 0, 0), "String", size=0)
        self.assertURL(ast(0, 0))


@unittest.skip("Disabled to avoid excessive network access")
class TestInlineCivetWithConfig(CivetTestCase):
    """Only !civet mergeresults needs real network access: it resolves via
    CivetExtension.hashes(), which is only populated by init()'s remotes-driven git lookup.
    The results/badges/report commands are covered without network by TestInlineCivet,
    TestBadgesNoNetwork, and TestReportNoNetwork respectively."""

    EXTENSIONS = [core, command, civet]
    MERGERESULTS = "[!civet!mergeresults]"

    def setupExtension(self, ext):
        if ext == civet:
            return dict(
                generate_test_reports=False,
                test_results_cache="/tmp/civet/jobs",
                remotes=dict(
                    moose=dict(url="https://civet.inl.gov", repo="idaholab/moose")
                ),
            )

    def testMergeResults(self):
        """!civet mergeresults; no need to render b/c it only uses core tokens"""

        ast = self.tokenize(self.MERGERESULTS)
        self.assertSize(ast, 1)
        self.assertToken(ast(0), "Paragraph", size=4)
        self.assertToken(ast(0, 0), "Link", size=1)
        self.assertToken(ast(0, 0, 0), "String", size=0)
        self.assertToken(ast(0, 1), "LineBreak", size=0)

        self.assertURL(ast(0, 0))
        self.assertToken(ast(0, 2), "Link", size=1)
        self.assertToken(ast(0, 2, 0), "String", size=0)
        self.assertURL(ast(0, 2))
        self.assertToken(ast(0, 3), "LineBreak", size=0)


class CivetNoNetworkTestCase(MooseDocsTestCase):
    """
    Common base for testing civet rendering with an injected, in-memory result database
    instead of "remotes" pointing at a real CIVET site, so these tests exercise the
    rendering path without depending on network access, which can be slow on some
    machines during local testing.
    """

    EXTENSIONS = [core, command, civet]
    FAKE_TNAME = "kernels.simple_diffusion.test"
    FAKE_TEST = civet_results.Test(
        "06_Test_-p_3", "OK", ["recover"], "", 0.53, "https://civet.inl.gov"
    )

    def injectFakeResults(self, tname):
        """Patch the civet extension's results() to return FAKE_TEST only for tname."""
        ext = next(e for e in self.translator.extensions if e.name == "civet")
        ext.results = lambda name: {12345: [self.FAKE_TEST]} if name == tname else {}


class TestBadgesNoNetwork(CivetNoNetworkTestCase):
    """
    Test civet badge rendering. The token is built via tokenize() and then has "prefix" set
    directly (as MooseDocs.extensions.sqa does: CivetTestBadges(item, prefix=req.prefix,
    tests=req.names)) since the markdown command itself doesn't expose "prefix".
    """

    BADGES = "[!civet!badges tests=simple_diffusion.test]"

    def testBadgesMaterialize(self):
        # Setup and tokenize in one call so the CivetExtension instance patched below is
        # the same one used by render() (a later tokenize/render call with args/kwargs would
        # rebuild the translator, and the extensions with it, from scratch).
        ast = self.tokenize(self.BADGES, renderer=base.MaterializeRenderer())
        ast(0, 0)["prefix"] = "kernels"
        self.injectFakeResults(self.FAKE_TNAME)

        res = self.render(ast)
        self.assertHTMLTag(res, "div", class_="moose-content")
        self.assertHTMLTag(res(0), "p", size=1)
        self.assertHTMLTag(res(0, 0), "div", size=1, class_="moose-civet-badges")
        self.assertHTMLTag(res(0, 0, 0), "span", size=1)
        self.assertHTMLTag(res(0, 0, 0, 0), "span", class_="new badge", string="1")
        self.assertEqual(res(0, 0, 0, 0)["data-badge-caption"], "OK")
        self.assertEqual(res(0, 0, 0, 0)["data-status"], "ok")


class TestReportNoNetwork(CivetNoNetworkTestCase):
    """
    Test civet test report rendering. Mirrors TestBadgesNoNetwork; "prefix" is set directly
    on the token since the markdown command itself doesn't expose it.
    """

    REPORT = "!civet report tests=simple_diffusion.test"

    def testReportMaterialize(self):
        ast = self.tokenize(self.REPORT, renderer=base.MaterializeRenderer())
        ast(0)["prefix"] = "kernels"
        self.injectFakeResults(self.FAKE_TNAME)

        res = self.render(ast)
        self.assertHTMLTag(res, "div", class_="moose-content")
        self.assertHTMLTag(res(0), "div", size=1, class_="moose-civet-test-report")
        self.assertHTMLTag(res(0, 0), "table")
        self.assertGreater(len(res(0, 0)), 1)


if __name__ == "__main__":
    unittest.main(verbosity=2)
