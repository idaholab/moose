"""
Align condition list in 'if' and 'while' clause if they are splitted in multiple lines.
All conditions should be aligned in the same column with the first condition.

== Violation ==

    if (a == b &&
      a == c)   <== Violation

== Good  ==

    if (a == b &&
        a == c) <== OK!
"""
from nsiqcppstyle_rulehelper import  *
from nsiqcppstyle_reporter import *
from nsiqcppstyle_rulemanager import *

def RunRule(lexer, contextStack) :

    t = lexer.GetCurToken()
    if t.type in ("IF", "WHILE") :
        t2 = lexer.GetNextTokenSkipWhiteSpaceAndCommentAndPreprocess()
        if t2 != None and t2.type == "LPAREN" :
            rparen = lexer.GetNextMatchingToken(True)
            firstElement = lexer.PeekNextTokenSkipWhiteSpaceAndCommentAndPreprocess()
            firstElementLineNo = firstElement.lineno
            firstElementColumn = GetRealColumn(firstElement)
            while(True) :
                t3 = lexer.GetNextTokenSkipWhiteSpaceAndCommentAndPreprocess()
                if t3 == None or t3 == rparen:
                    break
                if  firstElementLineNo != t3.lineno :
                    firstElementLineNo = t3.lineno
                    if firstElementColumn != GetRealColumn(t3) :
                        nsiqcppstyle_reporter.Error(t3, __name__, "Incorrect align on condition list '%s'. It should be aligned in column %d. " % (t3.value, firstElementColumn))

ruleManager.AddFunctionScopeRule(RunRule)







###########################################################################################
# Unit Test
###########################################################################################

from nsiqunittest.nsiqcppstyle_unittestbase import *
class testRule(nct):
    def setUpRule(self):
        ruleManager.AddFunctionScopeRule(RunRule)
    def test1(self):
        self.Analyze("test/thisFile.c",
"""
void function(int k, int j, int pp)
{
    if (AA == D &&
    kK = 22) {
    }
}
""")
        assert CheckErrorContent(__name__)
    def test2(self):
        self.Analyze("test/thisFile.c",
"""
void function(int k, int j, int pp)
{
    if (AA == D &&
        kK = 22) {
    }
}
""")
        assert not CheckErrorContent(__name__)
    def test3(self):
        self.Analyze("test/thisFile.c",
"""
void function(int k, int j, int pp)
{
    while (AA == D &&
        kK = 22) {
    }
}
""")
        assert  CheckErrorContent(__name__)
    def test4(self):
        self.Analyze("test/thisFile.c",
"""
while (AA == D &&
kK = 22) {
}
""")
        assert not CheckErrorContent(__name__)
    def test5(self):
        self.Analyze("test/thisFile.c",
"""
void F() {
    while (AA == D &&
           kK = 22
        ) {
    }
}
""")
        assert not CheckErrorContent(__name__)
