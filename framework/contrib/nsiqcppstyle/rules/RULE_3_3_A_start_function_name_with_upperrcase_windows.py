"""
Start the function name with a upper case letter.
It's rule for only Window C/C++ code.

== Violation ==

    bool checkSth() <== Violation. The function name starts with a lowercase 'c'.
    {
        return false;
    }

    bool _checkSth()  <== Violation. The function name starts with a lowercase 'c'.
    {
        return false;
    }

== Good ==

    bool IsSth()  <== OK.
    {
        return true;
    }

    bool _IsSth()  <== OK.
    {
    }
"""

from nsiqcppstyle_rulehelper import  *
from nsiqcppstyle_reporter import *
from nsiqcppstyle_rulemanager import *

keywords = ["_tWinMain", "_tmain"]

def RunRule(lexer, fullName, decl, contextStack, context) :

    t = lexer.GetCurToken()
    value = t.value
    if value.startswith("_") :
        value = value[1:]
    if value.startswith("~") :
        value = value[1:]
    if Search("^[a-z]", value) and not IsOperator(value) :
        if not t.value in keywords :

            nsiqcppstyle_reporter.Error(t, __name__, "Do not start function name(%s) with lowercase" % fullName)
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
bool CanHave() {
}""")
        assert not CheckErrorContent(__name__)
    def test2(self):
        self.Analyze("test/thisFile.c",
"""
bool CTEST:CanHave() {
}""")
        assert not CheckErrorContent(__name__)
    def test3(self):
        self.Analyze("test/thisFile.c",
"""
extern bool CTEST:canHave() {
}""")
        assert CheckErrorContent(__name__)
    def test4(self):
        self.Analyze("test/thisFile.c",
"""
extern int CTEST:_CanHave() {
}""")
        assert not CheckErrorContent(__name__)
    def test5(self):
        self.Analyze("test/thisFile.c",
"""
calss AA {
extern int ~IsIt();
}""")
        assert not CheckErrorContent(__name__)
    def test6(self):
        self.Analyze("test/thisFile.c",
"""
class K {
extern bool CTEST:canHave();
}""")
        assert  CheckErrorContent(__name__)

    def test7(self):
        self.Analyze("a.c",
"""
void ** *(d) (((int &,
  char **(*)(char *, char **));        // d is a pointer to a function that takes
""")
        assert not CheckErrorContent(__name__)

    def test8(self):
        self.Analyze("a.c",
"""
class A {
    void B() {
    void C() {
    }
}
""")
        assert not CheckErrorContent(__name__)
    def test9(self):
        self.Analyze("a.c",
"""
void operator=() {
}
""")
        assert not CheckErrorContent(__name__)

    def test10(self):
        self.Analyze("a.c",
"""
bool ConvertToTM(struct)
{
    int a= {0};
    memcpy(szTemp);
}
""")
        assert not CheckErrorContent(__name__)
    def test11(self):
        self.Analyze("a.c",
"""
bool& TT::operator=(struct)
{
    int a= {0};
    memcpy(szTemp);
}
""")
        assert not CheckErrorContent(__name__)
    def test12(self):
        self.Analyze("a.c",
"""
typedef c d();
""")
        assert CheckErrorContent(__name__)

    def test13(self):
        self.Analyze("test/thisFile.c",
"""
const std::string seasons = {
std::string("Spring"),
std::string("Summer"),
std::string("Autumn"),
std::string("Winter")
};
""")
        assert not CheckErrorContent(__name__)

    def test14(self):
        self.Analyze("test/thisFile.c",
"""
[event_source(native)]
interface ICamRecorder
{
}
""")
        assert not CheckErrorContent(__name__)

    def test15(self):
        self.Analyze("test/thisFile.c",
"""
void _tmain() {
}
void _tWinMain() {
}
""")
        assert not CheckErrorContent(__name__)

