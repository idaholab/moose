"""
Do not use uppercase letters for the c file.
This rule only applied on the only 'c' file.

== Violation ==

    /testdir/test_A1.c    <== Violation. Uppercase A is used.
    /testdir1/_TestBeta.c <== Violation. Uppercase T and B is used

== Good ==

    testdir/Test.cpp      <== Don't care. it's a cpp file.
    testdir1/test1.c      <== OK.
"""

from nsiqcppstyle_rulehelper import  *
from nsiqcppstyle_reporter import *
from nsiqcppstyle_rulemanager import *

def RunRule(lexer, filename, dirname) :
    if filename[filename.rfind("."):] == ".c" and Search("[A-Z]", filename)  :
        nsiqcppstyle_reporter.Error(DummyToken(lexer.filename, "", 0, 0), __name__,
              "Do not use uppercase for c file name (%s)." % filename)
ruleManager.AddFileStartRule(RunRule)








###########################################################################################
# Unit Test
###########################################################################################

from nsiqunittest.nsiqcppstyle_unittestbase import *
class testRule(nct):
    def setUpRule(self):
        ruleManager.AddFileStartRule(RunRule)

    def test1(self):
        self.Analyze("test/thisFile.c", "")
        assert CheckErrorContent(__name__)
    def test2(self):
        self.Analyze("test/ThisFile.cpp", "")
        assert not CheckErrorContent(__name__)

    def test3(self):
        self.Analyze("test/this_file.c", "")
        assert not CheckErrorContent(__name__)
