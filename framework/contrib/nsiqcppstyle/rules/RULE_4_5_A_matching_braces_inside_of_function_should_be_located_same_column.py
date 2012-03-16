"""
Matching braces inside of function definitions should be the same column.

== Violation ==

    void A() { <== Don't Care
        for (;;)
        { <== ERROR
          }
    }
    class K()
    { <== Don't Care
        if (true)
        { <== Error
      }
    }

== Good ==

    void A() { <== Don't Care
        for (;;)
        { <== Correct
        }
    }

"""
from nsiqcppstyle_rulehelper import  *
from nsiqcppstyle_reporter import *
from nsiqcppstyle_rulemanager import *

def RunRule(lexer, contextStack) :
    t = lexer.GetCurToken()
    if t.type == "RBRACE" and not t.pp:
        matching = lexer.GetPrevMatchingToken(True)
        if matching != None and t.lineno != matching.lineno :
            if GetRealColumn(t) != GetRealColumn(matching) :
                nsiqcppstyle_reporter.Error(t, __name__,
                      "Matching Braces inside of function should be located in the same column " )

ruleManager.AddFunctionScopeRule(RunRule)




###########################################################################################
# Unit Test
###########################################################################################

from nsiqunittest.nsiqcppstyle_unittestbase import *

class testRule(nct):
    def setUpRule(self):
        ruleManager.AddFunctionScopeRule(RunRule)
    def test1(self):
        self.Analyze("thisfile.c","""
void function() {
    for (;;) {
    }
}
""")
        assert CheckErrorContent(__name__)
    def test2(self):
        self.Analyze("thisfile.c","""
void function() {
    a = {
    }
}
""")
        assert CheckErrorContent(__name__)
    def test3(self):
        self.Analyze("thisfile.c","""
void function() {
    a = {
        }
    while(True)
    {
    }
    k = {}
}
""")
        assert not CheckErrorContent(__name__)
    def test4(self):
        self.Analyze("thisfile.c","""
void function() {
    for (;;) {
             }
}
""")
        assert not CheckErrorContent(__name__)
    def test5(self):
        self.Analyze("thisfile.c","""
void function() {
%sfor (;;) {
             }
}
""" % ('\t'))
        assert not CheckErrorContent(__name__)
    def test6(self):
        self.Analyze("thisfile.c","""
void function() {
void function2() {
for (;;)
{
  {
  }
}
}
""" )
        assert not CheckErrorContent(__name__)
