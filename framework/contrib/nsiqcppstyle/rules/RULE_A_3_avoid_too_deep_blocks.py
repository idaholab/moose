"""
Avoid too deep blocks.
If the block depth in the function is more than 4, it reports a violation.

== Violation ==

    void f() {
    {{{{{ <== Violation. Too deep. it's more than 4 blocks

    }}}}}
    }

== Good ==

    void f() {
    {{{{ <== OK!

    }}}}
    }


"""
from nsiqcppstyle_rulehelper import  *
from nsiqcppstyle_reporter import *
from nsiqcppstyle_rulemanager import *

depth = 0
reported = False
def RunRule(lexer, contextStack) :
    global depth
    global reported
    t = lexer.GetCurToken()
    if t.type == "LBRACE" :
        depth += 1
        if depth > 5 and not reported :
            nsiqcppstyle_reporter.Error(t, __name__, "Do not make too deep block(%d) ({). It makes not readable code" % depth)
            reported = True
    elif t.type == "RBRACE" :
        depth -= 1

def RunFunctionScopeRule(lexer, fullName, decl, contextStack, context) :
    global depth
    global reported
    reported = False
    depth = 0

ruleManager.AddFunctionNameRule(RunFunctionScopeRule)
ruleManager.AddFunctionScopeRule(RunRule)

###########################################################################################
# Unit Test
###########################################################################################

from nsiqunittest.nsiqcppstyle_unittestbase import *
class testRule(nct):
    def setUpRule(self):
        ruleManager.AddFunctionNameRule(RunFunctionScopeRule)
        ruleManager.AddFunctionScopeRule(RunRule)

    def test1(self):
        self.Analyze("thisfile.c",
"""
void func1() {
{{{{{{{
       }}}}}}}
}
""")
        assert CheckErrorContent(__name__)
    def test2(self):
        self.Analyze("thisfile.c",
"""

void func1() {
{{{
#define {{{{ }}}
       }}}}
}
""")
        assert not CheckErrorContent(__name__)

    def test3(self):
        self.Analyze("thisfile.c",
"""
void func(void)
{
if (...)
{ // depth-1

{ // depth-2

{ // depth-3

{ // depth-4

printf("...");

}
}
}
}
}
""")
        assert not CheckErrorContent(__name__)
    def test4(self):
        self.Analyze("thisfile.c",
"""
void func(void)
{
if (...)
{ // depth-1

{ // depth-2

{ // depth-3

{ // depth-4
{ // depth-5
printf("...");
}
}
}
}
}
}
""")
        assert CheckErrorContent(__name__)
