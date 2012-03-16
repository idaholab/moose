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

import os #@UnusedImport
import sys #@UnusedImport
import sre_compile
from nsiqcppstyle_util import * #@UnusedWildImport

class RuleManager :
    def __init__(self, runtimePath) :
        self.availRuleNames = []
        basePath = os.path.join(runtimePath, "rules")
        ruleFiles = os.listdir(basePath)
        rulePattern = sre_compile.compile("^(.*)\.py$")
        for eachRuleFile in ruleFiles :
            if os.path.isfile(os.path.join(basePath, eachRuleFile)) :
                ruleMatch = rulePattern.match(eachRuleFile)
                if ruleMatch != None and eachRuleFile.find("__init__") == -1 :
                    ruleName = ruleMatch.group(1)
                    self.availRuleNames.append(ruleName)
        self.availRuleCount = len(self.availRuleNames)
        self.availRuleModules = {}
        self.loadedRule = []
        self.rules = []
        self.preprocessRules = []
        self.functionNameRules = []
        self.functionScopeRules = []
        self.typeNameRules = []
        self.typeScopeRules = []
        self.lineRules = []
        self.fileEndRules = []
        self.fileStartRules = []
        self.projectRules = []
        self.rollBackImporter = None
#       self.LoadAllRules()


    def LoadRules(self, checkingRuleNames, printRule = True):
        """
        Load Rules. It resets rule before loading rules
        """
        self.ResetRules()
        self.ResetRegisteredRules()
        if self.rollBackImporter != None :
            self.rollBackImporter.uninstall()

        self.rollBackImporter = RollbackImporter()
        if printRule :
            print "======================================================================================"

        for ruleName in checkingRuleNames :
            count = self.availRuleNames.count(ruleName)
            if count == 0 :
                print "%s does not exist or incompatible." % ruleName
                continue
            else :
                if printRule :
                    print "  - ", ruleName, "is applied."
            ruleModule = __import__("rules."+ruleName)
            self.loadedRule.append(ruleModule)
        if len(self.loadedRule) == 0 :
            print "  No Rule is specified. Please configure rules in filefilter.txt."
        if printRule :
            print "======================================================================================"

    def ResetRules(self):
        self.loadedRule = []

    ############################################################################
    # Rule Runner
    ############################################################################
    def RunPreprocessRule(self, lexer, contextStack):
        """ Run rules which runs in the preprecessor blocks """
        for preprocessRule in self.preprocessRules :
            data = lexer.Backup()
            preprocessRule(lexer, contextStack)
            lexer.Restore(data)

    def RunFunctionNameRule(self, lexer, functionFullName, decl, contextStack, functionContext) :
        """ Run rules which runs on the function name """
        for eachFunctionNameRule in self.functionNameRules :
            data = lexer.Backup()
            eachFunctionNameRule(lexer, functionFullName, decl, contextStack, functionContext)
            lexer.Restore(data)

    def RunFunctionScopeRule(self, lexer, contextStack):
        """ Run rules which runs in the function blocks """
        for eachFunctionScopeRule in self.functionScopeRules :
            data = lexer.Backup()
            eachFunctionScopeRule(lexer,  contextStack)
            lexer.Restore(data)

    def RunTypeNameRule(self, lexer, typeName, typeFullName, decl, contextStack, typeContext):
        """ Run rules which runs on the type names """
        for typeNameRule in self.typeNameRules :
            data = lexer.Backup()
            typeNameRule(lexer, typeName, typeFullName, decl, contextStack, typeContext)
            lexer.Restore(data)

    def RunTypeScopeRule(self, lexer, contextStack):
        """ Run rules which runs in the type blocks """
        for typeScopeRule in self.typeScopeRules :
            data = lexer.Backup()
            typeScopeRule(lexer, contextStack)
            lexer.Restore(data)

    def RunRule(self, lexer, contextStack):
        """ Run rules which runs in any tokens """
        for rule in self.rules :
            data = lexer.Backup()
            rule(lexer, contextStack)
            lexer.Restore(data)

    def RunLineRule(self, lexer, line, lineno):
        """ Run rules which runs in each lines. """
        for lineRule in self.lineRules :
            data = lexer.Backup()
            lineRule(lexer, line, lineno)
            lexer.Restore(data)

    def RunFileEndRule(self, lexer, filename, dirname):
        """ Run rules which runs at the end of files. """
        for fileEndRule in self.fileEndRules :
            data = lexer.Backup()
            fileEndRule(lexer, filename, dirname)
            lexer.Restore(data)

    def RunFileStartRule(self, lexer, filename, dirname):
        """ Run rules which runs at the start of files. """
        for fileStartRule in self.fileStartRules :
            data = lexer.Backup()
            fileStartRule(lexer, filename, dirname)
            lexer.Restore(data)


    def RunProjectRules(self, targetName):
        """ Run rules which runs once a project. """
        for projectRule in self.projectRules :
            projectRule(targetName)


    ############################################################################
    # Rule Resister Methods
    ############################################################################
    def ResetRegisteredRules(self):
        """ Reset all registered rules. """
        del self.functionNameRules[:]
        del self.functionScopeRules[:]
        del self.lineRules[:]
        del self.rules[:]
        del self.typeNameRules[:]
        del self.typeScopeRules[:]
        del self.fileStartRules[:]
        del self.fileEndRules[:]
        del self.projectRules[:]
        del self.preprocessRules[:]

    def AddPreprocessRule(self, preprocessRule):
        """ Add rule which runs in preprocess statements """
        self.preprocessRules.append(preprocessRule)

    def AddFunctionScopeRule(self, functionScopeRule):
        """ Add rule which runs in function scope """
        self.functionScopeRules.append(functionScopeRule)


    def AddFunctionNameRule(self, functionRule):
        """ Add rule on the function name place"""
        self.functionNameRules.append(functionRule)

    def AddLineRule(self, lineRule):
        """ Add rule on the each line """
        self.lineRules.append(lineRule)

    def AddRule(self, rule):
        """ Add rule on any token """
        self.rules.append(rule)

    def AddTypeNameRule(self, typeNameRule):
        """ Add rule on any type (class / struct / union / namesapce / enum) """
        self.typeNameRules.append(typeNameRule)

    def AddTypeScopeRule(self, typeScopeRule):
        """ Add rule on the any type definition scope """
        self.typeScopeRules.append(typeScopeRule)

    def AddFileEndRule(self, fileEndRule):
        """
        Add rule on the file end
        Added Rule should be function with following prototype "def RunRule(lexer, filename, dirname)"
        lexer is the lexer used to analyze the source. it points the end token of source.
        filename is the filename analyzed.
        dirname is the file directory.
        """
        self.fileEndRules.append(fileEndRule)

    def AddFileStartRule(self, fileStartRule):
        """
        Add rule on the file start
        Added Rule should be function with following prototype "def RunRule(lexer, filename, dirname)"
        lexer is the lexer used to analyze the source. it points the start token of source.
        filename is the filename analyzed.
        dirname is the file directory.
        """
        self.fileStartRules.append(fileStartRule)

    def AddProjectRules(self, projectRule):
        """
        Add rule on the project
        Added Rule should be function with following prototype "def RunRule(targetName)"
        targetName is the analysis target directory.
        """
        self.projectRules.append(projectRule)

class RollbackImporter:
    def __init__(self):
        "Creates an instance and installs as the global importer"
        self.previousModules = sys.modules.copy()
        self.realImport = __builtins__["__import__"]
        __builtins__["__import__"] = self._import
        self.newModules = {}

    def _import(self, name, globals=None, locals=None, fromlist=[]):
        result = apply(self.realImport, (name, globals, locals, fromlist))
        if name.find("rules") != -1 :
            self.newModules[name] = 1
        return result

    def uninstall(self):
        for modname in self.newModules.keys():
            if modname.find("rules") != -1 :
                if not self.previousModules.has_key(modname):
                    # Force reload when modname next imported
                    del(sys.modules[modname])
        __builtins__["__import__"] = self.realImport


ruleManager = RuleManager(GetRuntimePath())

