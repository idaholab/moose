//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "ConsoleStreamInterface.h"
#include "MooseTypes.h"
#include "Syntax.h"

#include "hit.h"

#include <vector>
#include <string>
#include <iomanip>
#include <optional>

// Forward declarations
class ActionWarehouse;
class SyntaxTree;
class MooseApp;
class Factory;
class ActionFactory;
class GlobalParamsAction;
class JsonSyntaxTree;

class FuncParseEvaler : public hit::Evaler
{
public:
  virtual std::string
  eval(hit::Field * n, const std::list<std::string> & args, hit::BraceExpander & exp);
};

class UnitsConversionEvaler : public hit::Evaler
{
public:
  virtual std::string
  eval(hit::Field * n, const std::list<std::string> & args, hit::BraceExpander & exp);
};

class DupParamWalker : public hit::Walker
{
public:
  virtual void
  walk(const std::string & fullpath, const std::string & /*nodepath*/, hit::Node * n) override;

  std::vector<std::string> errors;

private:
  std::set<std::string> _duplicates;
  std::map<std::string, hit::Node *> _have;
};

class BadActiveWalker : public hit::Walker
{
public:
  virtual void walk(const std::string & /*fullpath*/,
                    const std::string & /*nodepath*/,
                    hit::Node * section) override;
  std::vector<std::string> errors;
};

class CompileParamWalker : public hit::Walker
{
public:
  typedef std::map<std::string, hit::Node *> ParamMap;
  CompileParamWalker(ParamMap & map) : _map(map){};

  virtual void
  walk(const std::string & fullpath, const std::string & /*nodepath*/, hit::Node * n) override;

private:
  ParamMap & _map;
};

class OverrideParamWalker : public hit::Walker
{
public:
  OverrideParamWalker(const CompileParamWalker::ParamMap & map) : _map(map) {}

  void walk(const std::string & fullpath, const std::string & /*nodepath*/, hit::Node * n) override;
  std::vector<std::string> warnings;

private:
  const CompileParamWalker::ParamMap & _map;
};

/**
 * Class for parsing input files. This class utilizes the GetPot library for actually tokenizing and
 * parsing files. It is not currently designed for extensibility. If you wish to build your own
 * parser, please contact the MOOSE team for guidance.
 */
class Parser
{
public:
  Parser();
  virtual ~Parser();

  /**
   * Parse an input file (or text string if provided) consisting of hit syntax and setup objects
   * in the MOOSE derived application
   */
  void parse(const std::vector<std::string> & input_filenames,
             const std::optional<std::string> & input_text = std::nullopt);

  /**
   * This function attempts to extract values from the input file based on the contents of
   * the passed parameters objects.  It handles a number of various types with dynamic casting
   * including vector types
   */
  void extractParams(const std::string & prefix, InputParameters & p);

  /**
   * Get the root pointer from front parser
   */
  hit::Node * getRootNode()
  {
    // normal input file is provided, front parser is set to parse input file
    if (_root)
      return _root.get();
    // no valid input file is provided, front parser is skipped
    else
      return nullptr;
  };

  /*
   * Get extracted variables from front parser
   */
  std::set<std::string> & getExtractedVars() { return _extracted_vars; }

  /*
   * Get input file names from parser
   */
  std::vector<std::string> & getInputFileNames() { return _input_filenames; }

protected:
  std::unique_ptr<hit::Node> _root = nullptr;
  std::unique_ptr<hit::Node> _cli_root = nullptr;

  /// The input file names that are used for parameter extraction
  std::vector<std::string> _input_filenames;

  /// The set of all variables extracted from the input file
  std::set<std::string> _extracted_vars;

  /// Boolean to indicate whether parsing has started (sections have been extracted)
  bool _sections_read;

private:
  std::string _errmsg;

  std::vector<std::string> _dw_errmsg;

  // Allow the MooseServer class to access the root node of the hit parse tree
  friend class MooseServer;
};
