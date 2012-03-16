"""
Provide space around word.
This rule checks if the spaces are provided before and after 'if', 'else', 'for' word
in the function scope
It doens't check 'switch' and 'while' because they are commonly attached following "("

== Violation ==

    void function()
    {
        for(k;j;c) { <== Violation. it should be 'for (k;j;c)'
        }
        if(k) { <== Violation. it should be 'if (k)'
        }else { <== Violation. it should be '} else'
        }
    }

== Good ==

    #define KK for(a;b;c) <== Don't care. It's not function scope.

    void function() {
        for (k;j;c) { <== OK
        }
        if (k) { <== OK
        } else { <== OK
        }
    }
"""

from nsiqcppstyle_rulehelper import  *
from nsiqcppstyle_reporter import *
from nsiqcppstyle_rulemanager import *

words = (
            "FOR",
            "ELSE",
            "IF",
)


def RunRule(lexer, contextStack) :
    t = lexer.GetCurToken()
    if t.type in words :
        t2 = lexer.PeekNextToken()
        t3 = lexer.PeekPrevToken()
        if t2 != None and t3 != None:
            if t2.type not in ["SPACE", "LINEFEED", "PREPROCESSORNEXT"] or t3.type not in ["SPACE", "LINEFEED"] :
                if not Search("^[ ]*#[ ]*include", t.line) :
                    nsiqcppstyle_reporter.Error(t, __name__,
                          "Put space before/after word '%s'." % t.value)

ruleManager.AddFunctionScopeRule(RunRule)
ruleManager.AddPreprocessRule(RunRule)
###########################################################################################
# Unit Test
###########################################################################################

from nsiqunittest.nsiqcppstyle_unittestbase import *
class testRule(nct):
    def setUpRule(self):
        ruleManager.AddFunctionScopeRule(RunRule)
        ruleManager.AddPreprocessRule(RunRule)
    def test1(self):
        self.Analyze("test/thisFile.c",
"""
void function(int k, int j, int pp)
{
  for(a;b;c) {
  }
}
""")
        assert CheckErrorContent(__name__)
    def test2(self):
        self.Analyze("test/thisFile.c",
"""
void function(int k, int j, int pp)
{
  if(k==3)
  {
  }
}
""")
        assert CheckErrorContent(__name__)
    def test3(self):
        self.Analyze("test/thisFile.c",
"""
void function(int k, int j, int pp)
{
  if (k==3)
  {
  }else {
  }
}
""")
        assert CheckErrorContent(__name__)
    def test4(self):
        self.Analyze("test/thisFile.c",
"""
if(k==3)
{
}
void function(int k, int j, int pp)
{
  if (k==3)
  {
  } else {
  }
  while(True) {
  }
  for (k;j; c) {
  }
}
""")
        assert not CheckErrorContent(__name__)

    def test5(self):
        self.Analyze("test/thisFile.c",
"""
#define AA do {\\
} while(0)
""")
        assert not CheckErrorContent(__name__)
    def test6(self):
        self.Analyze("test/thisFile.c",
"""
#define AA if\\
{} while(0)
""")
        assert not CheckErrorContent(__name__)
    def test7(self):
        self.Analyze("test/thisFile.c",
"""
#define AA if(\\
{} while(0)
""")
        assert CheckErrorContent(__name__)
    def test8(self):
        self.Analyze("test/thisFile.c",
"""
#  include <boost/preprocessor/repetition/for.hpp>
""")
        assert not CheckErrorContent(__name__)
