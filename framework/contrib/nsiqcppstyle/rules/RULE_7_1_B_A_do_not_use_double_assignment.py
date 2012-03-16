"""
Do not use double assignment.

if it's shown... this rule reports a violation.

== Violation ==

    k = t = 1;      <== Violation. double assignments are used.
    void a() {
        b = c = 2;  <== Violation. double assignments are used.
    }

== Good ==

    k = 1;  <== OK
    t = 1;
    void a() {
        b = 2;
        c = 2;
    }

"""
from nsiqcppstyle_rulehelper import  *
from nsiqcppstyle_reporter import *
from nsiqcppstyle_rulemanager import *

def RunRule(lexer, line, lineno) :
    if (Search(r"[a-zA-Z0-9]+\s*=\s*[a-zA-Z0-9]+\s*=", line)) :
        # Check it's not string. Followings are best guess....
        if line.count("\"") == 0 and not line.endswith("\\") :
            nsiqcppstyle_reporter.Error(DummyToken(lexer.filename, line, lineno, 0), __name__, "Do not use double assignment in a same line")

ruleManager.AddLineRule(RunRule)

###########################################################################################
# Unit Test
###########################################################################################

from nsiqunittest.nsiqcppstyle_unittestbase import *
class testRule(nct):
    def setUpRule(self):
        ruleManager.AddLineRule(RunRule)
    def test1(self):
        self.Analyze("thisfile.c","""
void Hello() {
   int k = a = 2;
}
""")
        assert CheckErrorContent(__name__)
    def test2(self):
        self.Analyze("thisfile.c","""
int k = c = 2;
void Hello() {
}
""")
        assert CheckErrorContent(__name__)
    def test3(self):
        self.Analyze("thisfile.c","""
int k = 2; int j = 2;
void Hello() {
}
""")
        assert not CheckErrorContent(__name__)
    def test4(self):
        self.Analyze("thisfile.c","""
a = "dsd=wew=e";
""")
        assert not CheckErrorContent(__name__)
    def test5(self):
        self.Analyze("thisfile.c","""

        l_Req.strGameInfoString = "P=&P0=YW55Y2FsbDQ0Mw==&P1=S19VTklUVEVTVA==&P2=MjU3&P3=aHR0cDovL2hhbmNkbi5oYW5nYW1lLmNvbS\
                              9wdWIvcGxpaS9hbHBoYQ==&P4=U3FmYWplbzUyVm1JMStzamg4emREc0lNWDMvQkppRzR5ekttNFNFQ09MdzJXSjV\
                              XUE54UlhIbzFKRzdPQnpnZUdnMWJoZFdvRUhOQU90WFlRN3d3VVdaR1RwbVBVV0hObW53YnB0U3Evb1NyVkEzbFNu\
                              WElXeEQrVGxyVERRZVlSNkovbGlLbG03MzNTSDRlSVFzM1d5QzArWW5HOTc3ZE16cGxUZCs0M1V5REtvK2lYREc1a\
                              HZsR1R3dXQ2Qk9pNVZaamxTSzFmQ1RUWjY0UWkyYzVXRVhDNHcyaVJrOElyMXZxbGRqdEhGNnZ2cmFSU1lGZUdNRl\
                              FmNzdiU2J5WkU3QTdQM1k2SjV0Z2U2clhrSUZYaFdmUi9mNEpGOFBGekt1OTJFMmxuK0JhaUxESWI5aVI5Rlp6VWR\
                              lWFJYellOZngvT0pQRnRDRXhaSlo2VElpVEJjYnI1SVNVa29TMA==&P5=bG9naW49QjQ4RTdFNEQ1QzFCNzQ5RUI1\
                              NTI5RCUyQyUyQ0FOWUNBTEwlMkNGJTJDMzklMkNZJTJDTENGMDAxX0hDRjAwMV9GTkYwMDFBX1BORjAwMSUyQ1klM\
                              kMqYW55Y2FsbDQ0MyUyQ0VDREYzOTEzMDI0RDI0OEVBNTQ1MjQxNTI4NjMlMkMlMkMlMkMlMkNOJTJDWSUyQ04lMk\
                              N0ZXN0JTQwZnJlZWNoYWwuY29tJTJDRkNDRTJGMDcxNjVGNzUlMkNMT0MwMDElMkMlMkMzJTJDWSUyQzAlN0MwJTd\
                              DbnVsbCUyQyUyQ05OJTIzTiUyM04lMkMlMkM7SEdfTE9HSU49UVNPUVY5SWpfY2s3V0tEWHJIOUctS0xOazlZOHFz\
                              a3NlXzRTYThvaWVacGlrOUF2cVA4SVA3NXYxaS0td1FCWnJLSGRZRUE0aU5Jc1VBUU1WWUxJNGFMYWNibHVia21fY\
                              TNwaEhvRE9Ta2xSNVV0MDd0emZFd1JqOW5jTDNQbFlMSXU0ZmYxT2JKMUNKdW1HWWtENlJjVWhKVWZMUTZHVWNlVE\
                              w4Tmxwd0dEWXhSYnZIak9maGhvUzlsRXIxekN1UTJSZURKa3pTZWlLNnFhVG9kUV81eWZRTno0REFaOVY0VXRaLTV\
                              DbmQyT0xwZ3FVWkwzT0lsQm9sWmQ1REdoMERPQUJRSFVuVWo3SHg3QUcwczduODNuYWNreVkxbXhtOXhCZE94dGRS\
                              RmljNDA0eTQ5bHNhY1M4QUxBZ09QWTc4V2xMYWlJNE9oRkJJNVpubmRGS1ZER3g4MkQ4aXFBbHFlM3lWLVpuRlRvP\
                              TtIR19DUF9MT0dJTj1lMmVvUzR2YlloU21rU1lKMFMzcWJVOTFjbjRjLU1td0xsZUNPQXo5bEcwZTVWdmNXeEZ0bD\
                              RyZFREQTF4WVV0Nll1T0xfVWtQNi1udDlQLWFlc3RncXdkdmRvRm9KMnlyNmE1b1dxQlVia2ozcUJyOGtaTnFWQ3d\
                              zUV9TSzI1TEtWZndxR2JtTkhwTU1Pczk0YWhNVXM3ZjdRWmVBZVVEMm52MVk0dTRMUGNreXhSZ1EzNVJyZER4UzBk\
                              TzZfRXdVRXNxRUNjY1V4ZmpaamRobEtvYUtQS19CNjhUay02LV9mMm5wQ2RGZDJrNDhXdHVROWxnaHFMbS1mdzNzT\
                              HdfMktsNjVGWC1uY3pVM0F6NVlUMFJ6UT09";
""")
        assert not CheckErrorContent(__name__)
