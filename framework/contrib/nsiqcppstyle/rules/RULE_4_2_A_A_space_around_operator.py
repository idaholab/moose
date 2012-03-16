"""
Provide the space before and after operators.
There are variation to provide spaces.
In the binary operator.
Spaces should be provided before and after the operator.
For example, +, /, %....

In the unary operator, Spaces should be provided before or after the operator.
However, when it's used in the (AA++), [--BB], {--KK}. It's OK not to provide spaces.

In the some operators(",", ";"), the spaces should be provided after the operator.


== Violation ==

    for (a;b;c)  <== Violation. It should be (a; b; c)
    Hello(a,b,c) <== Violation. It should be (a, b, c)
    int k = 2+3; <== Violation. It should be 2 + 3
    c+++c;       <== Violation. It should be c++ + c

== Good ==

    int k = (2 + 3); <== OK. It's ok not to provide spaces before and after [, (, {
    int k = -2;      <== OK. Minus can be used to indicate minus value.
                         Therefore This rule doesn't care about it.
    for (a; b; c) {} <== OK
    Hello(a, b, c);  <== OK
    tt[c++]          <== OK. This rule doesn't care about the unary operator is used in the [ ( [
"""

from nsiqcppstyle_rulehelper import  *
from nsiqcppstyle_reporter import *
from nsiqcppstyle_rulemanager import *

operator = (
            "PLUS",
            "DIVIDE",
            "MODULO",
            "OR",
            "LSHIFT",
            "LOR",
            "LAND",
            "LE",
            "GE",
            "EQ",
            'EQUALS',
            'TIMESEQUAL',
            'DIVEQUAL',
            'MODEQUAL',
            'PLUSEQUAL',
            'MINUSEQUAL',
            'LSHIFTEQUAL',
            'RSHIFTEQUAL',
            'ANDEQUAL',
            'XOREQUAL',
            'OREQUAL'

)

nextoperator = (
            "SEMI",
            "COMMA",
)

unaryoperator = (
        'PLUSPLUS', 'MINUSMINUS'
)
def RunRule(lexer, contextStack) :
    t = lexer.GetCurToken()

    if t.type in operator :
        t2 = lexer.PeekNextToken()
        t3 = lexer.PeekPrevToken()
        t4 = lexer.PeekPrevTokenSkipWhiteSpaceAndCommentAndPreprocess();
        if t2 != None and t3 != None  and (t4 == None or t4.type != "FUNCTION"):

            if t.pp == True and t.type == "DIVIDE":
                return
            if t2.type not in ["SPACE", "LINEFEED", "PREPROCESSORNEXT"] or t3.type not in ["SPACE", "LINEFEED"] :
                t3 = lexer.GetPrevTokenSkipWhiteSpaceAndComment()
                if t3 != None and t3.type != "OPERATOR" and not Match("^\w*#include", t.line):
                    nsiqcppstyle_reporter.Error(t, __name__,
                          "Provide spaces b/w operator '%s'" % t.value)
    elif t.type in nextoperator :
        t2 = lexer.PeekNextToken()
        if t2 != None and t2.type not in ["SPACE", "LINEFEED", "PREPROCESSORNEXT"] and not Match("^\w*#include", t.line):
            nsiqcppstyle_reporter.Error(t, __name__,
                          "Provide spaces after operator '%s'" % t.value)
    elif t.type in unaryoperator :
        t2 = lexer.PeekPrevToken()
        t3 = lexer.PeekNextToken()
        t4 = lexer.PeekPrevTokenSkipWhiteSpaceAndCommentAndPreprocess();
        if (Match("^\w*#include", t.line)) :
            return
        if (t3 != None and t3.type == "ID"):
            if t2.type not in ["COMMA", "OPERATOR", "SPACE", "LINEFEED", "LBRACE", "LPAREN", "LBRACKET"] and t3.type not in ["SEMI", "SPACE", "LINEFEED", "RBRACE", "RPAREN", "RBRACKET"]:
                nsiqcppstyle_reporter.Error(t, __name__,
                          "Provide spaces before operator '%s'" % t.value)

        if (t2 != None and t2.type == "ID"):
            if t3.type not in ["COMMA", "OPERATOR", "SPACE", "LINEFEED", "RBRACE", "RPAREN", "RBRACKET"] and t3.type not in ["SEMI", "SPACE", "LINEFEED", "RBRACE", "RPAREN", "RBRACKET"]:
                nsiqcppstyle_reporter.Error(t, __name__,
                          "Provide spaces after operator '%s'" % t.value)

ruleManager.AddRule(RunRule)
ruleManager.AddPreprocessRule(RunRule)







###########################################################################################
# Unit Test
###########################################################################################

from nsiqunittest.nsiqcppstyle_unittestbase import *
class testRule(nct):
    def setUpRule(self):
        ruleManager.AddRule(RunRule)
        ruleManager.AddPreprocessRule(RunRule)
    def test1(self):
        self.Analyze("test/thisFile.c",
"""
int *a;
void operator=(EWE) {
HELLO = ewe << 3;
TEST <= 3;
TEST < 3;
TEST | C;
TEST & C;
A != 3;
t = a++;
}
""")
        assert not CheckErrorContent(__name__)
    def test2(self):
        self.Analyze("test/thisFile.c",
"""
(DD +ww);
""")
        assert CheckErrorContent(__name__)
    def test3(self):
        self.Analyze("test/thisFile.c",
"""
HELLO = ewe <<3;
""")
        assert  CheckErrorContent(__name__)
    def test4(self):
        self.Analyze("test/thisFile.c",
"""
HELLo = TET ||B;
""")
        assert CheckErrorContent(__name__)
    def test5(self):
        self.Analyze("test/thisFile.c", "#define KK(dsd) TET ||B;")
        assert CheckErrorContent(__name__)
    def test6(self):
        self.Analyze("test/thisFile.c", "k = &b;")
        assert not CheckErrorContent(__name__)
    def test7(self):
        self.Analyze("test/thisFile.c", "k=b;")
        assert CheckErrorContent(__name__)
    def test8(self):
        self.Analyze("test/thisFile.c", "k|= b;")
        assert CheckErrorContent(__name__)
    def test9(self):
        self.Analyze("test/thisFile.c", "k++c;")
        assert CheckErrorContent(__name__)
    def test10(self):
        self.Analyze("test/thisFile.c", "#include <h/ds>")
        assert not CheckErrorContent(__name__)
    def test11(self):
        self.Analyze("test/thisFile.c", "hash ^= hash << 4;")
        assert not CheckErrorContent(__name__)

    def test12(self):
        self.Analyze("test/thisFile.c", """
#define KK() ewee;\\
hash ^= hash << 4;
""")
        assert not CheckErrorContent(__name__)

    def test13(self):
        self.Analyze("test/thisFile.c", """
#define KK() ewee;\\
hash ^= hash<<4;
""")
        assert CheckErrorContent(__name__)

    def test14(self):
        self.Analyze("test/thisFile.c", """
#include <magic++.h>
""")
        assert not CheckErrorContent(__name__)
    def test15(self):
        self.Analyze("test/thisFile.c", """
m_mTabCommand.SetAt(nId++, p##TabName##TabCommand);
""")
        assert not CheckErrorContent(__name__)

    def test16(self):
        self.Analyze("test/thisFile.c", """
m_mTabCommand.SetAt(++nId, p##TabName##TabCommand);
m_mTabCommand.SetAt(nId++dd);
""")
        assert  CheckErrorContent(__name__)
    def test17(self):
        self.Analyze("test/thisFile.c", """
string k = "k=b %s";
""")
        assert not CheckErrorContent(__name__)

    def test18(self):
        self.Analyze("test/thisFile.c", """
sprintf(l_szConfigPath, ""
"print%log");
""")
        assert not CheckErrorContent(__name__)

    def test19(self):
        self.Analyze("test/thisFile.c", """
sprintf(l_szConfigPath, "\\"
"print"
wewewe
wewe);
wewe
"ewewe"

""")
        assert not CheckErrorContent(__name__)

