"""
Do not use same filenames more than once.

== Vilolation ==

    /testdir/test1.c
    /testdir1/test1.c <== Violation. The filename 'test1' is used two times.

== Good ==

    testdir/test.c
    testdir1/test1.c
"""
from nsiqcppstyle_reporter import * #@UnusedWildImport
from nsiqcppstyle_rulemanager import * #@UnusedWildImport
import string

filenameMap = {}

def RunRule(lexer, filename, dirname) :
    if filename.startswith("stdafx.") :
        return
    filelist = filenameMap.get(filename, None)
    if filelist == None :
        filenameMap[filename] = []
        filenameMap[filename].append(os.path.join(dirname, filename))
    else :
        filenameMap[filename].append(os.path.join(dirname, filename))
        nsiqcppstyle_reporter.Error(DummyToken(lexer.filename, "", 0, 0), __name__,
          'Do not use same filename(%s) more than once. This filename is used in %s' % (filename, string.join(filenameMap[filename], ", ")))

ruleManager.AddFileStartRule(RunRule)




###########################################################################################
# Unit Test
###########################################################################################

from nsiqunittest.nsiqcppstyle_unittestbase import *
class testRule(nct):
    def setUpRule(self):
        ruleManager.AddFileStartRule(RunRule)
        global filenameMap
        filenameMap = {}

    def test1(self):
        self.Analyze("test/thisfile.c", "")
        self.Analyze("test2/thisfile.c", "")
        assert CheckErrorContent(__name__)

    def test2(self):
        self.Analyze("test/thisfile.c", "")
        self.Analyze("test/thisfile.h", "")
        assert not CheckErrorContent(__name__)

    def test3(self):
        self.Analyze("test/stdafx.h", "")
        self.Analyze("test/stdafx.h", "")
        self.Analyze("test/thisfile.c", "")
        self.Analyze("test/thisfile.h", "")
        assert not CheckErrorContent(__name__)
