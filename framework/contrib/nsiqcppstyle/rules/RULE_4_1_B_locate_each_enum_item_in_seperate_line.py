"""
Locate the each enum item in seperate lines.

== Violation ==

    enum A {
        A_A, A_B <== Violation
    }


== Good ==

    enum A {
        A_A,     <== Good
        A_B
    }
"""
from nsiqcppstyle_rulehelper import  *
from nsiqcppstyle_reporter import *
from nsiqcppstyle_rulemanager import *

def RunRule(lexer, typeName, typeFullName, decl, contextStack, typeContext) :
    if not decl and typeContext != None :
#        column = GetRealColumn(lexer.GetCurToken())
        if typeName == "ENUM" :
            lexer._MoveToToken(typeContext.startToken)
            while(True) :
                nt = lexer.GetNextTokenInTypeList(["COMMA", "RBRACE"], False, True)
                if nt == None or nt == typeContext.endToken : break
                if typeContext != nt.contextStack.Peek() : continue
                nt2 = lexer.PeekNextTokenSkipWhiteSpaceAndCommentAndPreprocess()
                nt3 = lexer.PeekPrevTokenSkipWhiteSpaceAndCommentAndPreprocess()
                #print nt, nt2,nt3
                if nt.lineno == nt2.lineno and nt3.lineno == nt.lineno:
                    nsiqcppstyle_reporter.Error(nt2, __name__, "Each enum item(%s) should be located in the different line" % nt2.value)
ruleManager.AddTypeNameRule(RunRule)




###########################################################################################
# Unit Test
###########################################################################################

from nsiqunittest.nsiqcppstyle_unittestbase import *
class testRule(nct):
    def setUpRule(self):
        ruleManager.AddTypeNameRule(RunRule)
    def test1(self):
        self.Analyze("test/thisFile.c",
"""
enum A {
}
""")
        assert not CheckErrorContent(__name__)
    def test2(self):
        self.Analyze("test/thisFile.c",
"""
enum C {
    AA, BB
}
""")
        assert CheckErrorContent(__name__)
    def test3(self):
        self.Analyze("test/thisFile.c",
"""
enum C {
    AA = 4,
    BB
}
""")
        assert not CheckErrorContent(__name__)
    def test4(self):
        self.Analyze("test/thisFile.c",
"""
enum C {
    AA = 4
    ,BB
}
""")
        assert not CheckErrorContent(__name__)
    def test5(self):
        self.Analyze("test/thisFile.c",
"""
enum C
{
    AA = 4
    ,BB
} TT;
""")
        assert not CheckErrorContent(__name__)

    def test6(self):
        self.Analyze("test/thisFile.c",
"""
enum COLOR
{
        COLOR_TRANSPARENT = RGB(0, 0, 255),
        COLOR_ROOM_IN_OUT = 0xffff00,
        COLOR_CHAT_ITEM = 0xff9419,
        COLOR_CHAT_MY = 0x00b4ff,
        COLOR_CHAT_YOUR = 0xa3d5ff,
        COLOR_ROOM_INFO = 0x00ffff,
        COLOR_RESULT_SCORE = 0xffcc00,
        COLOR_RESULT_RATING = 0x00fcff,
        COLOR_RESULT_POINT = 0x33ff00
}; """)
        assert not CheckErrorContent(__name__)
