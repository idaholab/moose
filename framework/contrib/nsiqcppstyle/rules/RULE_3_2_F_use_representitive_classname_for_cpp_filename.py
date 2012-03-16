"""
The file name should contain the representitive class/struct name.
If the file contains class/struct decls or defs, the file name should be
one of classes.
If the class/struct name starts with "C", "C" can be ommited in the file name.

== Vilolation ==

  = a.h =   <== Violation. It should contain class name 'TestClass'

    class TestClass() {
    }

  = a.cpp = <== Violation. It should contain class name 'Test'

    void Test::Method1() {
    }

== Good ==

  = TestClass.h = <== OK

    class TestClass {
    }

  = Class1.h = <== OK.

    class CClass1 {
    }

  = TestClass.cpp = <== OK

    void TestClass::Method1() {
    }


"""

from nsiqcppstyle_rulemanager import  *
from nsiqcppstyle_reporter import *
from nsiqcppstyle_rulemanager import *
try :
    set()
except NameError:
    from sets import Set as set

classname = None

def RunFunctionNameRule(lexer, fullName, decl, contextStack, context) :
    names = fullName.split("::")
    if len(names) > 1 :
        if len(names[0]) != 0 :
            classname.add(names[0])

def RunTypeNameRule(lexer, currentType, fullName, decl, contextStack, context) :
    if currentType in ["CLASS", "STRUCT"] :
        names = fullName.split("::")
        if len(names[-1]) != 0 :
            classname.add(names[-1])

def RunFileStartRule(lexer, filename, dirname) :
    global classname
    classname = set()

def RunFileEndRule(lexer, filename, dirname):
    goodFileName = False
    filename = filename.lower( )
    if len(classname) == 0 : return
    for t in classname :
        if t.startswith("C") :
            t = t[1:]
        if filename.find(t.lower()) != -1 :
            goodFileName = True
            break
    if not goodFileName :
        nsiqcppstyle_reporter.Error(DummyToken(lexer.filename, "", 0, 0), __name__,
                           "The filename does not represent the classnames (%s)" %(classname))

ruleManager.AddFileStartRule(RunFileStartRule)
ruleManager.AddTypeNameRule(RunTypeNameRule)
ruleManager.AddFunctionNameRule(RunFunctionNameRule)
ruleManager.AddFileEndRule(RunFileEndRule)


###########################################################################################
# Unit Test
###########################################################################################

from nsiqunittest.nsiqcppstyle_unittestbase import *
class testRule(nct):
    def setUpRule(self):
        ruleManager.AddFileStartRule(RunFileStartRule)
        ruleManager.AddTypeNameRule(RunTypeNameRule)
        ruleManager.AddFunctionNameRule(RunFunctionNameRule)
        ruleManager.AddFileEndRule(RunFileEndRule)

    def test1(self):
        self.Analyze("test/aa.c",
"""
void AA::DSD() {
}
""")
        assert not CheckErrorContent(__name__)
    def test2(self):
        self.Analyze("test/ab.c",
"""
void AA::DSD() {
}
""")
        assert CheckErrorContent(__name__)
    def test3(self):
        self.Analyze("test/aa.c",
"""
void CAA::DSD() {
}
""")
        assert not CheckErrorContent(__name__)
    def test4(self):
        self.Analyze("test/aa.c",
"""
void DSD() {
}
""")
        assert not CheckErrorContent(__name__)
    def test5(self):
        self.Analyze("test/aa.cpp",
"""
struct AA {
}

class BB {
}
""")
        assert not CheckErrorContent(__name__)
    def test6(self):
        self.Analyze("test/aa.cpp",
"""
struct AA1 {
}

class BB {
}
""")
        assert  CheckErrorContent(__name__)
    def test7(self):
        self.Analyze("test/CamRecorderFactory.cpp",
"""
class __declspec(dllexport) CCamRecorderFactory
{
};
""")
        assert not  CheckErrorContent(__name__)

    def test8(self):
        self.Analyze("test/CamRecorderFactory.cpp",
"""
class DLLEXPORT CCamRecorderFactory
{
};
""")
        assert not  CheckErrorContent(__name__)
