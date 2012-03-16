"""
align long function parameters on the first parameter when it's defined in multiple lines.


== Violation ==

    void functionA(int a, int b
              int c); <== Violation.

    void functionB(int a, int c,
                       int d) <== Violation

== Good ==

    void functionA(int a, int b
                   int c); <== OK.

"""
from nsiqcppstyle_rulehelper import  *
from nsiqcppstyle_reporter import *
from nsiqcppstyle_rulemanager import *


def RunRule(lexer, fullName, decl, contextStack, context) :
    lexer.GetNextTokenInType("LPAREN", False, True)
    rparen = lexer.GetNextMatchingToken(True)
    firstElement = lexer.PeekNextTokenSkipWhiteSpaceAndCommentAndPreprocess()
    firstElementLineNo = firstElement.lineno
    firstElementColumn = GetRealColumn(firstElement)
    while(True) :
        t = lexer.GetNextTokenSkipWhiteSpaceAndCommentAndPreprocess()
        if t == None or t == rparen:
            break
        if  firstElementLineNo != t.lineno :
            firstElementLineNo = t.lineno
            if firstElementColumn != GetRealColumn(t) :
                nsiqcppstyle_reporter.Error(t, __name__, "Incorrect align on long parameter list in front of '%s', it should be aligen in column %d." % (t.value, firstElementColumn))


ruleManager.AddFunctionNameRule(RunRule)





###########################################################################################
# Unit Test
###########################################################################################

from nsiqunittest.nsiqcppstyle_unittestbase import *
class testRule(nct):
    def setUpRule(self):
        ruleManager.AddFunctionNameRule(RunRule)
    def test1(self):
        self.Analyze("test/thisFile.c",
"""
void function(int k, int j
              int pp)
{
}
""")
        assert not CheckErrorContent(__name__)
    def test2(self):
        self.Analyze("test/thisFile.c",
"""
void function(int k, int j,
             int pp)
{
}
""")
        assert  CheckErrorContent(__name__)
    def test3(self):
        self.Analyze("test/thisFile.c",
"""
void function(int k, int j,

             int pp)
{
}
""")
        assert CheckErrorContent(__name__)
    def test4(self):
        self.Analyze("test/thisFile.c",
"""
void function(int k, int j, int pp)
{
}
""")
        assert not CheckErrorContent(__name__)
    def test5(self):
        self.Analyze("test/thisFile.c",
"""
class A {
void function(int k, int j,
              int pp);
}
""")
        assert not CheckErrorContent(__name__)
    def test6(self):
        self.Analyze("test/thisFile.c",
"""
class A {
void function(int k, int j,
            int pp);
}
""")
        assert  CheckErrorContent(__name__)
    def test7(self):
        self.Analyze("test/thisFile.c",
"""
class A {
void function(int k, int j,
              int pp)
{
    function(KK, DD,
             TT);
}
}
""")
        assert not CheckErrorContent(__name__)
    def test8(self):
        self.Analyze("test/thisFile.c",
"""
class A {
void function(int k, int j,
              int pp)
{
    function(KK, DD,
             TT);
}
}
""")
        assert not CheckErrorContent(__name__)
    def test9(self):
        self.Analyze("test/thisFile.c",
"""
Void aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa( int a,
                                          int b)
{
}
""")
        assert not CheckErrorContent(__name__)


    def test10(self):
        self.Analyze("test/thisFile.c",
"""
OrgDNSHandler::RESULT_CODE OrgDNSHandler::process( const NRootDNSConfig* pNRootDNSConfig,
                                                   const nano::Variant::List& params,
                                                   int a)
{
    return NULL;
};

void functionA(int a, int b
               int c);
""")
        assert not CheckErrorContent(__name__)
