"""
Do not use unberbars for cpp filename.

Only alphabets, numbers can be used for a cpp filename.

== Vilolation ==

    /testdir/test_1.cpp <== Violation. - is used.
    /testdir1/_test1.cpp <== Violation. _ is used

== Good ==

    testdir/test.cpp
    testdir1/test_1.c <== Don't care. it's c file.
"""

from nsiqcppstyle_rulehelper import  *
from nsiqcppstyle_reporter import *
from nsiqcppstyle_rulemanager import *

def RunRule(lexer, filename, dirname) :
    if Search("[_]", filename) and filename[filename.rfind("."):] in (".cpp", ".cxx") :
        nsiqcppstyle_reporter.Error(DummyToken(lexer.filename, "", 0,0), __name__,
                           "Do not use underbar for cpp file name (%s)." % filename)

ruleManager.AddFileStartRule(RunRule)








###########################################################################################
# Unit Test
###########################################################################################

from nsiqunittest.nsiqcppstyle_unittestbase import *
class testRule(nct):
    def setUpRule(self):
        ruleManager.AddFileStartRule(RunRule)

    def test1(self):
        self.Analyze("test/this_file.cpp", "")
        assert CheckErrorContent(__name__)
    def test2(self):
        self.Analyze("test/thisfile.cpp", "")
        assert not CheckErrorContent(__name__)

    def test3(self):
        self.Analyze("test/this_file.c", "")
        assert not CheckErrorContent(__name__)

