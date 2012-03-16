"""
Do not ommit function parameter names in the function declaration.
It checks function decls only.

== Violation ==

    void functionA(int a, int); <== Violation. The second parameter int has no name.

    void functionB(int );  <== Violation. The first parameter in has no name

== Good ==

    void functionA(int a, int b, int c, int d, int e); <== Good.

    void functionB(int, int, int c, int d) <== Don't care. it's the function definition.
    {
    }
"""

from nsiqcppstyle_rulehelper import  *
from nsiqcppstyle_reporter import *
from nsiqcppstyle_rulemanager import *


def RunRule(lexer, fullName, decl, contextStack, context) :
    if decl :
        t2 = lexer.GetCurToken()
        lexer.GetNextTokenInType("LPAREN", False, True)
        lexer.PushTokenIndex()
        rparen = lexer.GetNextMatchingToken()
        lexer.PopTokenIndex()
        count = 0

        while(True) :
            t = lexer.GetNextTokenSkipWhiteSpaceAndCommentAndPreprocess()
            if rparen == None or t == rparen or t == None :
                break
            if t.type in ["ID", "BOOL", "CHAR", "INT", "LONG", "DOUBLE", "FLOAT", "SHORT", "VOID"] :
                if t.type == "VOID" :
                    nt = lexer.PeekNextTokenSkipWhiteSpaceAndCommentAndPreprocess()
                    if nt == rparen :
                        return
                count += 1
            elif t.type == "LT" :
                lexer.GetNextMatchingGT()
            elif t.type == "COMMA"  :
                if count == 1 :
                    nsiqcppstyle_reporter.Error(t2, __name__, "function (%s) has non named parameter. use named parameter." % fullName)
                    break;
                count = 0
            elif rparen.lexpos <= t.lexpos :
                if count == 1 :
                    nsiqcppstyle_reporter.Error(t2, __name__, "function (%s) has non named parameter. use named parameter." % fullName)
                    break;

ruleManager.AddFunctionNameRule(RunRule)








###########################################################################################
# Unit Test
###########################################################################################


from nsiqunittest.nsiqcppstyle_unittestbase import *
class testRule(nct):
    def setUpRule(self):
        ruleManager.AddFunctionNameRule(RunRule)
    def test1(self):
        self.Analyze("thisfile.c",
"""
int functionA(int *a, K<a, b>, int b, int c, int c);
""")
        assert CheckErrorContent(__name__)
    def test2(self):
        self.Analyze("thisfile.c",
"""
int functionA(int, int, int, Scope<T,J> a) {
}

int B;
""")
        assert not CheckErrorContent(__name__)
    def test3(self):
        self.Analyze("thisfile.c",
"""
class K {
int functionA(int *a, int, int, tt&b, aa*s, k a);
int B;
}
""")
        assert CheckErrorContent(__name__)
    def test4(self):
        self.Analyze("thisfile.c",
"""
class K {
int functionA(int *a, int c, int d, tt&b, aa*s, k a);
int B;
}
""")
        assert not CheckErrorContent(__name__)

    def test5(self):
        self.Analyze("thisfile.c",
"""
class K {
int functionA(void);
int B;
}
""")
        assert not CheckErrorContent(__name__)
    def test6(self):
        self.Analyze("thisfile.c",
"""
class K {
int functionA(void*);
int B;
}
""")
        assert not CheckErrorContent(__name__)

    def test7(self):
        self.Analyze("thisfile.c",
"""
#include <stdio.h>
#include <sys/socket.h>                // getpeername()

#define ILOG_WARN(...) \\
        iota::BoxLog::Instance().WriteFormat(box::Warn, __FILE__, __LINE__, __VA_ARGS__)


void func(void)
{
    if (::getpeername(nFileDescriptor, (struct sockaddr*) &oSockAddr, (socklen_t*) &nSockAddrSize) == -1)
    {
        int        nErrorCode = errno;

        ILOG_WARN("Initialize() - internal error. (getpeername) : \n\t\t"
                            "* this=[%p], fd=[%d], \n\t\t"
                            "* error-code=[%d], error-message=[%s]",
                    this, nFileDescriptor, nErrorCode, strerror(nErrorCode));
        return false;
    }
}

""")
        assert not CheckErrorContent(__name__)
    def test8(self):
        self.Analyze("thisfile.c",
"""
#define ILOG_WARN(A) \\
        iota::BoxLog::Instance().WriteFormat(box::Warn, __FILE__, __LINE__, __VA_ARGS__)
""")
        assert not CheckErrorContent(__name__)

    def test9(self) :

        self.Analyze("thisfile.c",
"""
/**
 * @brief constructor with map
 */
ExeOptionDetail& ExeOptionDetail::operator=(const nano::Variant::Map& mapOptions)
{
    m_nTimeout = _getItemAsInt(mapOptions, "TIMEOUT", -1);
    return *this;
};

""")
        assert not CheckErrorContent(__name__)

    def test10(self):
        self.Analyze("thisfile.c",
"""struct FnVisibility
{
void operator () (const DSObjMap::value_type& pair)
{
DSObject*    pObject = pair.second;
CHTMLDomUtility::SetStyleProperty(pObject, _T("display"), _T("none"));    // ==> Original Code : Rule_6_1_A_Error
}
};
""")
        assert not CheckErrorContent(__name__)

    def test11(self):
        self.Analyze("thisfile.c",
"""
CPoint BtnTeamPos[]    = { CPoint(BTN_SINGLE_POS_X, BTN_SINGLE_POS_Y),
                             CPoint(BTN_TEAM_A_POS_X, BTN_TEAM_A_POS_Y),
                            CPoint(BTN_TEAM_B_POS_X, BTN_TEAM_B_POS_Y),
                            CPoint(BTN_TEAM_C_POS_X, BTN_TEAM_C_POS_Y)
                        };
""")
        assert not CheckErrorContent(__name__)

