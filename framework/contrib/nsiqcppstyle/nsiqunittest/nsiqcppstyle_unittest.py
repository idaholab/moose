# Copyright (c) 2009 NHN Inc. All rights reserved.
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#    * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#    * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#    * Neither the name of NHN Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import unittest
import nsiqcppstyle_checker
import nsiqcppstyle_state
class urlTest(unittest.TestCase):
    def test2(self):
        data = """
#ifdef __NAME__
#define KK
void function() {
}
auto
#if 0
    void function2() {
    }
#endif
#endif
"""
        navigator = nsiqcppstyle_checker.CppLexerNavigator("a.cpp", data)

        while(True) :
            tok = navigator.GetNextToken()
            if tok == None : break

    def test3(self):
        data = """
#ifdef dsd
void function1() {
#else
void function2() {
#endif
}
"""
        navigator = nsiqcppstyle_checker.CppLexerNavigator("a.cpp", data)
        nsiqcppstyle_checker.ContructContextInfo(navigator)
        navigator.Reset()
        while(True) :
            tok = navigator.GetNextTokenSkipWhiteSpaceAndComment()
            if tok == None : break
            print tok, tok.contextStack
    def test4(self):
        data = """
#define dsd(dsd) \\
\tdo { \\
   ff(1);\\
} while(0)
int a;
"""

        nsiqcppstyle_state._nsiqcppstyle_state.verbose = True
        navigator = nsiqcppstyle_checker.CppLexerNavigator("a.cpp", data)
        nsiqcppstyle_checker.ContructContextInfo(navigator)
        navigator.Reset()
        while(True) :
            tok = navigator.GetNextTokenSkipWhiteSpaceAndComment()
            if tok == None : break
            print tok, tok.contextStack, tok.pp

