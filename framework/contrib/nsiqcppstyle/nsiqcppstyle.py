#!/usr/bin/python
#
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
#
import sys
import os
import csv #@UnusedImport
import urllib #@UnusedImport
import urllib2 #@UnusedImport
try:
    import hashlib #@UnusedImport
except ImportError:
    import md5 #@UnusedImport
import unittest #@UnusedImport
import platform #@UnusedImport
import sre_compile #@UnusedImport
import shutil #@UnusedImport

def WeAreFrozen():
    return hasattr(sys, "frozen")

def ModulePath():
    if WeAreFrozen():
        return os.path.dirname(unicode(sys.executable, sys.getfilesystemencoding()))
    return os.path.dirname(unicode(__file__, sys.getfilesystemencoding()))

def GetRuntimePath() :
    "Return the path of this tool"
    if (sys.platform == "win32") :
        runtimePath = ModulePath();
    else :
        modename = globals()['__name__']
        module = sys.modules[modename]
        runtimePath = os.path.dirname(module.__file__)
    return runtimePath


if __name__ == "__main__":
    sys.path.append(GetRuntimePath())
    module = __import__("nsiqcppstyle_exe")
    sys.exit(module.main())
