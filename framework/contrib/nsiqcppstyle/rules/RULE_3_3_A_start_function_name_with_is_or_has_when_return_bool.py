"""
Start the function name with 'is' or 'has' when returning bool.
This rule might be violated a lot because there are a lot of cases
in which functions return bool to indicate exceptions.
Please turn off this rule. If you think it's too overwhelming.

== Violation ==

    bool checkSth() { <== Violation. The function name should be isSth or hasSth.
        return false;
    }

== Good ==

    bool isSth() { <== OK.
        return true;
    }

    is isSth() { <== Don't care. it's not returnning bool.
    }
"""

from nsiqcppstyle_rulehelper import  *
from nsiqcppstyle_reporter import *
from nsiqcppstyle_rulemanager import *

def RunRule(lexer, fullName, decl, contextStack, context) :
    t = lexer.GetCurToken()
    functionName = t.value.lower()
    k = 0
    t2 = None
    while(True) :
        t2 = lexer.GetPrevTokenSkipWhiteSpaceAndCommentAndPreprocess()
        if t2 == None or k > 4 or t2.type == "SEMI":
            break;
        if t2.value.lower() == "bool" :
            if not Search("^(has|is)", functionName) and functionName != "operator" :
                nsiqcppstyle_reporter.Error(t, __name__,
                       "The function name(%s) should start with has or is when returinning bool" % fullName)
            break;
        k += 1

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
bool canHave() {
}""")
        assert CheckErrorContent(__name__)
    def test2(self):
        self.Analyze("test/thisFile.c",
"""
bool CTEST:canHave() {
}""")
        assert CheckErrorContent(__name__)
    def test3(self):
        self.Analyze("test/thisFile.c",
"""
extern bool CTEST:canHave() {
}""")
        assert CheckErrorContent(__name__)
    def test4(self):
        self.Analyze("test/thisFile.c",
"""
extern int CTEST:canHave() {
}""")
        assert not CheckErrorContent(__name__)
    def test5(self):
        self.Analyze("test/thisFile.c",
"""
extern int CTEST:isIt() {
}""")
        assert not CheckErrorContent(__name__)
    def test6(self):
        self.Analyze("test/thisFile.c",
"""
class K {
extern bool CTEST:canHave();
}""")
        assert CheckErrorContent(__name__)

    def test7(self):
        self.Analyze("test/thisFile.c", """
/**
              *          Check if the requesting is necessary.
              */
             bool IsSetToRequest() const;


             /// Gates
             /**
              *          Add the exit gate item.
              */
             void AddGate(GATE gate){m_GateCont.push_back(gate); }
""")
        assert not CheckErrorContent(__name__)

    def test8(self):
        self.Analyze("test/thisFile.c",
"""
boolean operator=();
boolean KK::operator=();
""")
        assert not CheckErrorContent(__name__)
