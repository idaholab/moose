"""
Do not use hardcoded paths in #include.

== Violation ==

    #include "c:\\Hello.h" <== Violation. Window style absolute path.
    #include "/usr/include/Hello.h" <== Violation. Linux style absolute path.

== Good ==

    #include "Hello.h"
    #include "include/Hello.h"
"""

from nsiqcppstyle_rulehelper import  *
from nsiqcppstyle_reporter import *
from nsiqcppstyle_rulemanager import *

def RunRule(lexer, contextStack) :
    t = lexer.GetCurToken()
    if t.type == "PREPROCESSOR" and t.value.find("include") != -1 :
        d = lexer.GetNextTokenSkipWhiteSpaceAndComment()
        if d != None and d.type == "STRING" :
            value = d.value
            if value.startswith("\"/") or Search(r"^\"[a-zA-Z]:", value) :
                nsiqcppstyle_reporter.Error(d, __name__, "Do not use absolute path(%s) in the include path" % value)

ruleManager.AddPreprocessRule(RunRule)


###########################################################################################
# Unit Test
###########################################################################################

from nsiqunittest.nsiqcppstyle_unittestbase import *
class testRule(nct):
    def setUpRule(self):
        ruleManager.AddPreprocessRule(RunRule)
    def test1(self):
        self.Analyze("thisfile.c","""# include "c:\k.h"
void func1()
{}""")
        assert CheckErrorContent(__name__)
    def test2(self):
        self.Analyze("thisfile.c", """
#include "/ewe/dsd" """)
        assert CheckErrorContent(__name__)
    def test3(self):
        self.Analyze("thisfile.c","""
#include "ewe\kk" """)
        assert not CheckErrorContent(__name__)
    def test4(self):
        self.Analyze("thisfile.c","""
#include </ewe/kk> """)
        assert not CheckErrorContent(__name__)
