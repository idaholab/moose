"""
Provide the function doxygen comment.
It detects if there is doxygen sytle comments in front of each functions in the header.
It only checks non private funcions.

== Violation ==

 = a.h =
    void FunctionA();  <== Violation. No doxygen comment.

    /*        <== Violation. It's not a doxygen comment
     *
     */
    void FunctionB();

== Good ==

   = a.h =
    /**
     * blar blar
     */
    void FunctionA(); <== OK

    /**
     * blar
     */
    void FunctionB() {  <== OK.
    }

    class A {
        private :
            void FunctionC(); <== Don't care. it's private function.
    }
  = a.c =
     void FunctionD(); <== Don't care. it's defined in c file.
"""

from nsiqcppstyle_rulehelper import  *
from nsiqcppstyle_rulemanager import *


def RunRule(lexer, fullName, decl, contextStack, context) :
    ext = lexer.filename[lexer.filename.rfind("."):]
    if ext == ".h" :
        upperBlock = contextStack.SigPeek()
        if upperBlock != None and upperBlock.type == "CLASS_BLOCK" and upperBlock.additional == "PRIVATE":
            return

        t = lexer.GetCurToken()

        lexer.PushTokenIndex()
        t2 = lexer.GetPrevTokenInType("COMMENT")
        lexer.PopTokenIndex()
        lexer.PushTokenIndex()
        t3 = lexer.GetPrevTokenInTypeList(["SEMI", "PREPROCESSOR"], False, True)
        lexer.PopTokenIndex()
        if t2 != None and t2.additional == "DOXYGEN" :
            if t3 == None or t2.lexpos > t3.lexpos :
                return
        nsiqcppstyle_reporter.Error(t, __name__,
              "Doxygen Comment should be provided in front of function (%s) in header." % fullName)
ruleManager.AddFunctionNameRule(RunRule)


def RunTypeScopeRule(lexer, contextStack) :
    t = lexer.GetCurToken()
    if t.type in ["PUBLIC", "PRIVATE", "PROTECTED"] :
        curContext = contextStack.SigPeek()
        if curContext.type in ["CLASS_BLOCK", "STRUCT_BLOCK"]:
            curContext.additional = t.type

ruleManager.AddTypeScopeRule(RunTypeScopeRule)




###########################################################################################
# Unit Test
###########################################################################################

from nsiqunittest.nsiqcppstyle_unittestbase import *
class testRule(nct):
    def setUpRule(self):
        ruleManager.AddFunctionNameRule(RunRule)
        ruleManager.AddTypeScopeRule(RunTypeScopeRule)
    def test1(self):
        self.Analyze("thisfile.h",
"""
void FunctionA();
""")
        assert CheckErrorContent(__name__)
    def test2(self):
        self.Analyze("thisfile.h",
"""
/*
 *
 */
extern void FunctionB();
""")
        assert  CheckErrorContent(__name__)
    def test3(self):
        self.Analyze("thisfile.h",
"""
class A {
public:
    void ~A();
}
""")
        assert  CheckErrorContent(__name__)
    def test4(self):
        self.Analyze("thisfile.h",
"""
class J {
public :
    /** HELLO */
    A();
private :
    B();
    C() {
    }
}
""")
        assert not CheckErrorContent(__name__)
    def test5(self):
        self.Analyze("thisfile.h",
"""
/*
 *
 */
 void FunctionB() {
}
""")
        assert  CheckErrorContent(__name__)

    def test6(self):
        self.Analyze("thisfile.h",
"""
int a;
 void FunctionB();
""")
        assert  CheckErrorContent(__name__)

