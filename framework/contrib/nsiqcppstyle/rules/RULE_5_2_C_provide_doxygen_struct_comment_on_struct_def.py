"""
Provide the struct/union doxygen comment.
It checks if there is doxygen sytle comment in front of each struct/union definition.

== Violation ==

    struct A { <== Violation. No doxygen comment.
    };

    /*        <== Violation. It's not doxygen comment
     *
     */
    union B {
    };

== Good ==

    /**
     * blar blar
     */
    struct A { <== OK
    };

    struct A; <== Don't care. It's forward decl.
"""

from nsiqcppstyle_rulehelper import  *
from nsiqcppstyle_reporter import *
from nsiqcppstyle_rulemanager import *


def RunRule(lexer, currentType, fullName, decl, contextStack, context) :
    if not decl and currentType in ("STRUCT", "UNION")  and context != None:
        t = lexer.GetCurToken()
        lexer.PushTokenIndex()
        t2 = lexer.GetPrevTokenInType("COMMENT")
        lexer.PopTokenIndex()
        lexer.PushTokenIndex()
        t3 = lexer.GetPrevTokenInTypeList(["SEMI", "PREPROCESSOR", "LBRACE"], False, True)
        lexer.PopTokenIndex()
        if t2 != None and t2.additional == "DOXYGEN" :
            if t3 == None or t2.lexpos > t3.lexpos :
                return
        nsiqcppstyle_reporter.Error(t, __name__, "Doxygen Comment should be provided in front of struct/union def(%s)." % fullName)
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
struct A {
}
""")
        assert CheckErrorContent(__name__)
    def test2(self):
        self.Analyze("thisfile.c",
"""
/*
 */
struct K {
}
""")
        assert  CheckErrorContent(__name__)
    def test3(self):
        self.Analyze("thisfile.c",
"""
/**
 */
struct K {
    struct T {
    }
}
""")
        assert  CheckErrorContent(__name__)
    def test4(self):
        self.Analyze("thisfile.c",
"""
/**
 *
 */
struct J {
    int k;
    /**
     */
    struct T {
    }
}
class T;
""")
        assert not CheckErrorContent(__name__)
    def test5(self):
        self.Analyze("thisfile.c",
"""
/*
 */
struct K {
}
""")
        assert CheckErrorContent(__name__)
    def test6(self):
        self.Analyze("thisfile.c",
"""
typedef struct  {
} K
""")
        assert CheckErrorContent(__name__)
