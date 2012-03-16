"""
Do not use the special characters in a filename.
Only alphabets, numbers and underbars can be used for a filename.

== Vilolation ==

    /testdir/test-1.c <== Violation. - is used.
    /testdir1/test!1.c <== Violation. ! is used

== Good ==

    testdir/test.c
    testdir1/test_1.c
"""
from nsiqcppstyle_reporter import * #@UnusedWildImport
from nsiqcppstyle_rulemanager import * #@UnusedWildImport
from nsiqcppstyle_checker import * #@UnusedWildImport
def RunRule(lexer, filename, dirname) :
    if not Match(r"^[_A-Za-z0-9\.]*$", filename) :
        nsiqcppstyle_reporter.Error(DummyToken(lexer.filename, "", 0, 0), __name__,
          'Do not use special characters in file name (%s).' % filename)

ruleManager.AddFileStartRule(RunRule)






###########################################################################################
# Unit Test
###########################################################################################

from nsiqunittest.nsiqcppstyle_unittestbase import *
class testRule(nct):
    def setUpRule(self):
        ruleManager.AddFileStartRule(RunRule)

    def test1(self):
        self.Analyze("test/this-file.c", "")
        self.Analyze("test2/!thisfile22.c", "")
        assert CheckErrorContent(__name__)

    def test2(self):
        self.Analyze("test/thisfile.c", "")
        self.Analyze("test/thisfile.h", "")
        assert not CheckErrorContent(__name__)
