"""
Braces for namespace should be located in the seperate line.

== Violation ==

    namespace AA { <== ERROR
    }



== Good ==

    namespace
    {
    }

"""
from nsiqcppstyle_rulehelper import  *
from nsiqcppstyle_reporter import *

from nsiqcppstyle_rulehelper import  *
from nsiqcppstyle_reporter import *
from nsiqcppstyle_rulemanager import *


def RunRule(lexer, currentType, fullName, decl, contextStack, typeContext) :
    if not decl and currentType == "NAMESPACE" and typeContext != None:
        t = lexer.GetNextTokenInType("LBRACE", False, True)
        if t != None :
            t2 = typeContext.endToken
            if t2 != None and t.lineno != t2.lineno :
                prevToken = lexer.GetPrevTokenSkipWhiteSpaceAndCommentAndPreprocess()
                #print contextStack.Peek()
                if prevToken != None and prevToken.lineno == t.lineno :
                    nsiqcppstyle_reporter.Error(t, __name__, "The brace for type definition should be located in start of line")
                if  t2.lineno != t.lineno and GetRealColumn(t2) != GetRealColumn(t) :
                    nsiqcppstyle_reporter.Error(t2, __name__, "The brace for type definition should be located in same column")

ruleManager.AddTypeNameRule(RunRule)






###########################################################################################
# Unit Test
###########################################################################################

from nsiqunittest.nsiqcppstyle_unittestbase import *

class testRule(nct):
    def setUpRule(self):
        ruleManager.AddTypeNameRule(RunRule)
    def test1(self):
        self.Analyze("thisfile.c","""
public class A {

}
""")
        assert not CheckErrorContent(__name__)
    def test2(self):
        self.Analyze("thisfile.c","""
class C : public AA {

}
""")
        assert not CheckErrorContent(__name__)
    def test3(self):
        self.Analyze("thisfile.c","""
class K
{
    void function() const {
    }
    class T
    {
    }
}
""")
        assert not CheckErrorContent(__name__)
    def test4(self):
        self.Analyze("thisfile.c","""
namespace K
{
    void function() const {
    }
    class T {
    }
}
""")
        assert not CheckErrorContent(__name__)
    def test5(self):
        self.Analyze("thisfile.c","""
namespace K {
    int k;
}
""")
        assert CheckErrorContent(__name__)

