"""
Do not use more than 5 parameters in each function.
Use struct instead.
It checks function decl and impl together.
This rule is confused when the multiple template(KK<T, K>) is used in the parameter.
When you have to use such sentence, please // NS in the end of line to ignore this error.

== Violation ==

    void functionA(int a, int b, int c, int d, int e, int j); <== Violated, more than 5 parameters.

    void functionB(int a, int b, int c, int d, int e, int j, int k)  <== Violated
    {
    }

== Good ==

    void functionA(int a, int b, int c, int d, int e); <== Good. 5 parameters.

    void functionB(int a, int b, int c, int d) <== Good. 4 parameters
    {
    }

"""
from nsiqcppstyle_rulehelper import  *
from nsiqcppstyle_reporter import *
from nsiqcppstyle_rulemanager import *

def RunRule(lexer, fullName, decl, contextStack, context) :
    lexer.GetNextTokenInType("LPAREN", False, True)
    lexer.PushTokenIndex()
    rparen = lexer.GetNextMatchingToken()
    lexer.PopTokenIndex()
    count = 0
    while(True) :
        t = lexer.GetNextToken(True, True, True, True)
        if t.type == "COMMA"  :
            count += 1
        elif rparen == t :
            if count >= 5 :
                nsiqcppstyle_reporter.Error(t, __name__, "function (%s) has more than 5 parameters. please use struct instead." % fullName)
            break;

ruleManager.AddFunctionNameRule(RunRule)







###########################################################################################
# Unit Test
###########################################################################################

from nsiqunittest.nsiqcppstyle_unittestbase import *
class testRule(nct):
    def setUpRule(self):
        ruleManager.AddFunctionNameRule(RunRule)
    def test1(self):
        self.Analyze("thisfile.c",
"""
int functionA(int *a, int b, int c, int d, Scope<T,J> a) {
}
""")
        assert CheckErrorContent(__name__)
    def test2(self):
        self.Analyze("thisfile.c",
"""
int functionA(int *a, int b, int c,   Scope<T,J> a) {
}
""")
        assert not CheckErrorContent(__name__)
    def test3(self):
        self.Analyze("thisfile.c",
"""
int functionA(int *a, int b, int c, tt&b, aa*s, k a) {
}
""")
        assert CheckErrorContent(__name__)
    def test4(self):
        self.Analyze("thisfile.c",
"""
class T {
int functionA(int *a, int b, int c, tt&b, aa*s, k a) {
}
};
""")
        assert CheckErrorContent(__name__)
