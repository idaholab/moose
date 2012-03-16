"""
Provide the class doxygen comment.
It checks if there is doxygen sytle comment in front of each class definition.

== Violation ==

    class A { <== Violation. No doxygen comment.
    };

    /*        <== Violation. It's not a doxygen comment
     *
     */
    class B {
    };

== Good ==

    /**
     * blar blar
     */
    class A { <== OK
    };

    class B; <== Don't care. It's forward decl.
"""
from nsiqcppstyle_rulehelper import  *
from nsiqcppstyle_reporter import *
from nsiqcppstyle_rulemanager import *


def RunRule(lexer, currentType, fullName, decl, contextStack, typeContext) :
    if not decl and currentType == "CLASS"  and typeContext != None:
        t = lexer.GetCurToken()
        lexer.PushTokenIndex()
        t2 = lexer.GetPrevTokenInType("COMMENT")
        lexer.PopTokenIndex()
        lexer.PushTokenIndex()
        t3 = lexer.GetPrevTokenInTypeList(["LBRACE", "SEMI", "PREPROCESSOR"], False, True)
        lexer.PopTokenIndex()
        if t2 != None and t2.additional == "DOXYGEN" :
            if t3 == None or t2.lexpos > t3.lexpos :
                return
        nsiqcppstyle_reporter.Error(t, __name__, "Doxygen Comment should be provided in front of class def(%s)." % fullName)
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
class A {
}
""")
        assert CheckErrorContent(__name__)
    def test2(self):
        self.Analyze("thisfile.c",
"""
/*
 */
class K {
}
""")
        assert  CheckErrorContent(__name__)
    def test3(self):
        self.Analyze("thisfile.c",
"""
/**
 */
class K {
    class T {
    }
}
""")
        assert  CheckErrorContent(__name__)

        assert  CheckErrorContent(__name__)
    def test4(self):
        self.Analyze("thisfile.c",
"""
/**
 *
 */
class J {
    int k;
    /**
     */
    class T {
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
        assert not CheckErrorContent(__name__)
    def test6(self):
        self.Analyze("thisfile.c",
"""
/**
 */
template<class A, class B>
class K {
}
""")
        assert not CheckErrorContent(__name__)
