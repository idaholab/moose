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

import sre_compile
import nsiqcppstyle_state

_regexp_compile_cache = {}

def Match(pattern, s):
    """Matches the string with the pattern, caching the compiled regexp."""
    # The regexp compilation caching is inlined in both Match and Search for
    # performance reasons; factoring it out into a separate function turns out
    # to be noticeably expensive.
    if not pattern in _regexp_compile_cache:
        _regexp_compile_cache[pattern] = sre_compile.compile(pattern)
    return _regexp_compile_cache[pattern].match(s)


def Search(pattern, s):
    """Searches the string for the pattern, caching the compiled regexp."""
    if not pattern in _regexp_compile_cache:
        _regexp_compile_cache[pattern] = sre_compile.compile(pattern)
    return _regexp_compile_cache[pattern].search(s)

def FindAll(pattern, s):
    """Searches the string for the pattern, caching the compiled regexp."""
    if not pattern in _regexp_compile_cache:
        _regexp_compile_cache[pattern] = sre_compile.compile(pattern)
    return _regexp_compile_cache[pattern].findall(s)

def GetRealColumn(token):
    """ Get the token's real column """
    tabsize = int(nsiqcppstyle_state._nsiqcppstyle_state.GetVar("tabsize", 4))

    line = token.line[:token.column]
    tabCount = line.count("\t")
    return len(line) + tabCount * (tabsize - 1)

def GetIndentation(token):
    """ Get indentation of the line in which the tokens exists"""
    tabsize = int(nsiqcppstyle_state._nsiqcppstyle_state.GetVar("tabsize", 4))

    line = token.line
    indent  = 0
    for char in line:
        if char == ' ' : indent += 1
        elif char == '\t' : indent += tabsize
        else : break;
    return  indent

def IsConstuctor(value, fullName, context):
    """ Check if the passed value is the contructor or destructor """
    fullName = fullName.replace("~", "")
    names = fullName.split("::")
    if len(names) != 1 and names[-1] == value :
        return True

    if context != None and context.type in ["CLASS_BLOCK", "STRUCT_BLOCK"] :
        names = context.name.split("::")
        if names[-1] == value :
            return True
    return False

def IsOperator(value):
    """ Check if the passed value is 'operator' """
    if value != None and value == "operator" :
        return True
    return False
