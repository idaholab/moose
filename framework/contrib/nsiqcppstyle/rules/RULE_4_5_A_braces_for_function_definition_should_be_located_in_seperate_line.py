"""
Braces for function definition should be located in the seperate line.

== Violation ==

    void A() { <== Violation

    }

    void A()
    {
      } <== Violation. It has different columns.

== Good ==

    void A()
    { <== OK
    }

    void K()
    {
       while(True) { <== Don't care
       }
    }

"""
from nsiqcppstyle_rulehelper import  *
from nsiqcppstyle_reporter import *
from nsiqcppstyle_rulemanager import *


def RunRule(lexer, fullName, decl, contextStack, typeContext) :
    if not decl and typeContext != None :
        t = lexer.GetNextTokenInType("LBRACE", False, True)
        if t != None :
            t2 = typeContext.endToken
            if t2 != None and t.lineno != t2.lineno :
                prevToken = lexer.GetPrevTokenSkipWhiteSpaceAndCommentAndPreprocess()
                #print contextStack.Peek()
                if prevToken != None and prevToken.lineno == t.lineno :
                    nsiqcppstyle_reporter.Error(t, __name__, "The brace for function definition should be located in start of line")
                if t2.lineno != t.lineno and GetRealColumn(t2) != GetRealColumn(t) :
                    nsiqcppstyle_reporter.Error(t2, __name__, "The brace for function definition should be located in same column")

ruleManager.AddFunctionNameRule(RunRule)

###########################################################################################
# Unit Test
###########################################################################################

from nsiqunittest.nsiqcppstyle_unittestbase import *

class testRule(nct):
    def setUpRule(self):
        ruleManager.AddFunctionNameRule(RunRule)
    def test1(self):
        self.Analyze("thisfile.c","""
void function() {

}
""")
        assert CheckErrorContent(__name__)
    def test2(self):
        self.Analyze("thisfile.c","""
void function() const {

}
""")
        assert CheckErrorContent(__name__)
    def test3(self):
        self.Analyze("thisfile.c","""
class K {
    void function() const
    {

    }
}
""")
        assert not CheckErrorContent(__name__)
    def test4(self):
        self.Analyze("thisfile.c","""
void function()
{
    while(True) {
    }
}
class A {
void function()
  {
  }
}
""")
        assert not CheckErrorContent(__name__)
    def test5(self):
        self.Analyze("thisfile.c","""
class K {
    void function() const
    {   }
}
""")
