"""
Do not use buffer overflow risky functions in unix env.
if they're found, this rule reports a volation.

== Buffer Overflow Risky Function List ==
    - strcpy()
    - strcat()
    - sprintf()
    - vsprintf()
    - gets()
    - realpath()
    - getopt()
    - getpass()
    - streadd()
    - strecpy()
    - strtrns()
"""

from nsiqcppstyle_rulehelper import  *
from nsiqcppstyle_rulemanager import *
import nsiqcppstyle_reporter
unix_bufferoverflow_functions = (
  'strcpy',
  'strcat',
  'sprintf',
  'vsprintf',
  'gets',
  'realpath',
  'getopt',
  'getpass',
  'streadd',
  'strecpy',
  'strtrns'
)
def RunRule(lexer, contextStack) :
    t = lexer.GetCurToken()
    if t.type == "ID" :
        if t.value in unix_bufferoverflow_functions :
            t2 = lexer.PeekNextTokenSkipWhiteSpaceAndComment()
            if t2 != None and t2.type == "LPAREN" :
                t3 = lexer.PeekPrevTokenSkipWhiteSpaceAndComment()
                if t3 == None or t3.type != "PERIOD" :
                    nsiqcppstyle_reporter.Error(t, __name__,
                      "Do not use burfferoverflow risky function(%s)" % t.value)

ruleManager.AddFunctionScopeRule(RunRule)

###########################################################################################
# Unit Test
###########################################################################################

from nsiqunittest.nsiqcppstyle_unittestbase import *
class testRule(nct):
    def setUpRule(self):
        ruleManager.AddFunctionScopeRule(RunRule)

    def test1(self):
        self.Analyze("thisfile.c",
"""
void func1()
{
    k = strcat()
}
""")
        assert CheckErrorContent(__name__)
    def test2(self):
        self.Analyze("thisfile.c",
"""

void func1() {
#define strcat() k
}
""")
        assert not CheckErrorContent(__name__)
    def test3(self):
        self.Analyze("thisfile.c",
"""
void strcat() {
}
""")
        assert not CheckErrorContent(__name__)
    def test4(self):
        self.Analyze("thisfile.c",
"""
void strcat () {
}
""")
        assert not CheckErrorContent(__name__)
    def test5(self):
        self.Analyze("thisfile.c",
"""
void func1()
{
    k = help.strcat ()
}
""")
        assert not CheckErrorContent(__name__)
