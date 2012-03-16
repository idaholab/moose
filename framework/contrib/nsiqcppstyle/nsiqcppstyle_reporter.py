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

import nsiqcppstyle_state
import nsiqcppstyle_checker
import nsiqcppstyle_rulemanager
import sys
import csv
import os

####################################################################################################
csvfile = None
writer = None
target = None
def PrepareReport(outputPath, format) :
    """
    Set up sth like report headers
    """
    global writer
    if format == "csv" :
        if os.path.isdir(outputPath) :
            outputPath = os.path.join(outputPath, "nsiqcppstyle_report.csv")
        csvfile = file(outputPath, "wb")
        writer = csv.writer(csvfile)
        writer.writerow(("File", "Line", "Column", "Message", "Rule", "Rule Url"))
    elif format == "xml" :
        if os.path.isdir(outputPath) :
            outputPath = os.path.join(outputPath, "nsiqcppstyle_report.xml")
        writer = file(outputPath, "wb")
        writer.write("<?xml version='1.0'?>\n<checkstyle version='4.4'>\n")
    elif format == "html" :
        if os.path.isdir(outputPath) :
            outputPath = os.path.join(outputPath, "nsiqcppstyle_report.html")
        writer = file(outputPath, "wb")
        writer.write("<!DOCTYPE html><html><head><font size=2><ul>\n")

def ReportSummaryToScreen(analyzedFiles, nsiqcppstyle_state, filter, ciMode) :
    """
    Report Summary Info into the screen.
    """
    if _nsiqcppstyle_state.quiet == True:
        if _nsiqcppstyle_state.output_format == 'html':
            global writer
            fileCount = len(analyzedFiles)
            violatedFileCount = len(nsiqcppstyle_state.errorPerFile.keys())
            buildQuality = 0
            if fileCount != 0 :
                buildQuality = (fileCount - violatedFileCount) * 100.0 / fileCount
            writer.write("""<hr /><br>================================== Summary Report ===================================<br> ** Total Available Rules     : %d<br> ** Total Applied Rules       : %d<br> ** Total Violated Rules      : %d<br> ** Total Errors Occurs       : %d<br> ** Total Analyzed Files      : %d<br> ** Total Violated Files Count: %d<br> ** Build Quality             : %.2f%%<br><hr />""" % (nsiqcppstyle_rulemanager.ruleManager.availRuleCount, len(nsiqcppstyle_state.checkers), len(nsiqcppstyle_state.errorPerChecker.keys()), nsiqcppstyle_state.error_count, len(analyzedFiles), violatedFileCount, buildQuality))
    else:
        fileCount = len(analyzedFiles)
        violatedFileCount = len(nsiqcppstyle_state.errorPerFile.keys())
        buildQuality = 0
        if fileCount != 0 :
            buildQuality = (fileCount - violatedFileCount) * 100.0 / fileCount
        print "\n"
        print ("=================================== Summary Report ===================================");
        print (" ** Total Available Rules     : %d" % nsiqcppstyle_rulemanager.ruleManager.availRuleCount)
        print (" ** Total Applied Rules       : %d" % len(nsiqcppstyle_state.checkers))
        print (" ** Total Violated Rules      : %d" % len(nsiqcppstyle_state.errorPerChecker.keys()))
        print (" ** Total Errors Occurs       : %d" % nsiqcppstyle_state.error_count)
        print (" ** Total Analyzed Files      : %d" % len(analyzedFiles))
        print (" ** Total Violated Files Count: %d" % violatedFileCount)
        print (" ** Build Quality             : %.2f%%" % buildQuality)
        if not ciMode :
            print ("\n================================ Violated Rule Details ===============================")
            for checker in nsiqcppstyle_state.errorPerChecker.keys() :
                print " - ", checker, "rule violated :", nsiqcppstyle_state.errorPerChecker[checker]
            print ("\n================================ Violated File Details ===============================")
            for eachFile in nsiqcppstyle_state.errorPerFile.keys() :
                count = 0
                for  eachRule in nsiqcppstyle_state.errorPerFile[eachFile].keys() :
                    count += nsiqcppstyle_state.errorPerFile[eachFile][eachRule]
                    print " - ", eachFile, " violated in total : ", count
                for  eachRule in nsiqcppstyle_state.errorPerFile[eachFile].keys() :
                    print "   * ", eachRule, " : ", nsiqcppstyle_state.errorPerFile[eachFile][eachRule]
def CloseReport(format) :
    if format == "xml" :
        global writer
        writer.write("</checkstyle>\n")
        writer.close()
    if format == "html" :
        writer.write("</ul></font></head></html>\n")
        writer.close()
####################################################################################################

#ruleMap = {}
def IsRuleUsed(ruleName, ruleNames) :
    if ruleNames.count(ruleName) == 0 :
        return "false"
    else : return "true"

def ReportRules(availRuleName, ruleNames):
    pass
    #global ruleMap
    #ruleMap = {}
    #index = 0
    #===========================================================================
    # for eachAvailRuleName in availRuleName :
    #    ruleMap[eachAvailRuleName] = index
    #    index += 1
    #
    # if _nsiqcppstyle_state.output_format == 'xml':
    #    writer.write("<rules>\n")
    #    for eachAvailRuleName in availRuleName :
    #        url = "http://nsiqcppstyle.appspot.com/rule_doc/" + eachAvailRuleName
    #        writer.write("<rule name='%s' use='%s' index='%d' ruleDoc='%s'/>\n" % (eachAvailRuleName, IsRuleUsed(eachAvailRuleName, ruleNames), ruleMap[eachAvailRuleName], url ))
    #    writer.write("</rules>\n")
    #===========================================================================

def StartDir(dirname):
    if _nsiqcppstyle_state.output_format == 'xml':
        pass
# writer.write("<dir name='%s'>\n" % (dirname))

def EndDir():
    if _nsiqcppstyle_state.output_format == 'xml':
        pass
        # writer.write("</dir>\n")

def StartTarget(targetname):
    """ Write Report when each target is analyzed"""
    if _nsiqcppstyle_state.output_format == 'xml':
        global target
        target = targetname
#  writer.write("<target name='%s'>\n" % (targetname))

def EndTarget():
    """ Write Report when each target is ended"""
    if _nsiqcppstyle_state.output_format == 'xml':
        pass #writer.write("</target>\n")


def StartFile(dirname, filename):
    if _nsiqcppstyle_state.output_format == 'xml':
        writer.write("<file name='%s'>\n" % (os.path.join(target,dirname[1:], filename)))

def EndFile():
    if _nsiqcppstyle_state.output_format == 'xml':
        writer.write("</file>\n")



_nsiqcppstyle_state =  nsiqcppstyle_state._nsiqcppstyle_state

def __dict_replace(s, d):
    """Replace substrings of a string using a dictionary."""
    for key, value in d.items():
        s = s.replace(key, value)
    return s

def escape(data, entities={}):
    """Escape &, <, and > in a string of data.

    You can escape other strings of data by passing a dictionary as
    the optional entities parameter.  The keys and values must all be
    strings; each key will be replaced with its corresponding value.
    """

    # must do ampersand first
    data = data.replace("&", "&amp;")
    data = data.replace(">", "&gt;")
    data = data.replace("<", "&lt;")
    if entities:
        data = __dict_replace(data, entities)
    return data



def ErrorInternal(t, ruleName, message):
    """
    Print error
    """
    global rule
    ruleName = ruleName[6:]

    if t == None :
        return
    if  nsiqcppstyle_checker.Search(r"//\s*NS", t.line) == None and not _nsiqcppstyle_state.CheckRuleSuppression(ruleName):
        _nsiqcppstyle_state.IncrementErrorCount(ruleName, t.filename)
        url = ""
        if _nsiqcppstyle_state.showUrl :
            url = "http://nsiqcppstyle.appspot.com/rule_doc/" + ruleName
        if _nsiqcppstyle_state.output_format == 'emacs':
            sys.stdout.write('%s:%s:  %s  [%s] %s\n' % (
              t.filename, t.lineno, message, ruleName, url))
        elif _nsiqcppstyle_state.output_format == 'vs7':
            sys.stdout.write('%s(%s, %s):  %s  [%s] %s\n' % (
              t.filename, t.lineno, t.column, message, ruleName, url))
        elif _nsiqcppstyle_state.output_format == 'eclipse':
            sys.stdout.write('  File "%s", line %d %s (%s)\n' %(t.filename, t.lineno, message, ruleName))
        elif _nsiqcppstyle_state.output_format == 'csv':
            global writer
            writer.writerow((t.filename, t.lineno, t.column, message, ruleName, url))
        elif _nsiqcppstyle_state.output_format == 'xml':
            writer.write("""<error line='%d' col='%d' severity='warning' message='%s' source='%s'/>\n""" % (t.lineno, t.column, escape(message).replace("'", "\""), ruleName))
        elif _nsiqcppstyle_state.output_format == 'html':
            if _nsiqcppstyle_state.baseURL != '':
                writer.write("""<li><a href=%s/%s>%s</a><ul><li>Line: %d - Column: %d</li><li>Rule: %s</li></ul>\n""" % (_nsiqcppstyle_state.baseURL, t.filename[t.filename.find(_nsiqcppstyle_state.base_dir):], t.filename[t.filename.find(_nsiqcppstyle_state.base_dir):].split('/').pop(), t.lineno, t.column, escape(message).replace("'", "\"")))
            else:
                writer.write("""<li><a href=%s/%s>%s</a><ul><li>Line: %d - Column: %d</li><li>Rule: %s</li></ul>\n""" % (_nsiqcppstyle_state.baseURL, t.filename[t.filename.find(_nsiqcppstyle_state.base_dir):], t.filename[t.filename.find(_nsiqcppstyle_state.base_dir):].split('/').pop(), t.lineno, t.column, escape(message).replace("'", "\"")))

Error = ErrorInternal

class DummyToken:
    def __init__(self, filename, line, lineno, column):
        self.filename = filename
        self.line = line

        if lineno == 0 :
            lineno = 1
        self.lineno = lineno
        self.column = column
