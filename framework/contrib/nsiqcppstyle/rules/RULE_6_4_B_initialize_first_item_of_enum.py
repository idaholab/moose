"""
Check the first item of enum type and check if it is initalized.

It checks the first element and see there is = token.

== Violation ==

    enum A {
        A, B <== Violation
    }

== Good ==

    enum A {
        A=4, B <== OK
    }

"""
from nsiqcppstyle_rulehelper import  *
from nsiqcppstyle_reporter import *
from nsiqcppstyle_rulemanager import *


def RunRule(lexer, typeName, typeFullName, decl, contextStack, typeContext) :
    if not decl and typeName == "ENUM" and typeContext != None :
        lexer._MoveToToken(typeContext.startToken)
        t2 = lexer.GetNextTokenSkipWhiteSpaceAndCommentAndPreprocess()
        t3 = lexer.GetNextTokenSkipWhiteSpaceAndCommentAndPreprocess()
        if t3 != None and t3.type != "EQUALS" :
            nsiqcppstyle_reporter.Error(t3, __name__,
                  "The first item(%s) of enum type(%s) should be initialized." % (t2.value, typeFullName))

ruleManager.AddTypeNameRule(RunRule)



###########################################################################################
# Unit Test
###########################################################################################


from nsiqunittest.nsiqcppstyle_unittestbase import *
class testRule(nct):
    def setUpRule(self):
        ruleManager.AddTypeNameRule(RunRule)
    def test1(self):
        self.Analyze("thisfile.c",
"""
enum KK {
    tt,
    kk
}
""")
        assert  CheckErrorContent(__name__)
    def test2(self):
        self.Analyze("thisfile.c",
"""
enum KK {
    tt = 1,
    kk
}
""")
        assert not CheckErrorContent(__name__)
    def test3(self):
        self.Analyze("thisfile.c",
"""
enum KK {
    tt = 1, kk
}
""")
        assert not CheckErrorContent(__name__)
    def test4(self):
        self.Analyze("thisfile.c",
"""
typedef enum {
    tt = 1, kk
} KK;
""")
        assert not CheckErrorContent(__name__)
    def test5(self):
        self.Analyze("thisfile.c",
"""
void A() {
enum KK{
    tt, kk
};
}
""")
        assert  CheckErrorContent(__name__)


