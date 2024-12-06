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

#include "hit/hit.h"

#include <vector>
#include <string>
#include <iomanip>
#include <optional>
#include <filesystem>

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
  /**
   * Constructor given a list of input files, given in \p input_filenames.
   *
   * Optionally, the file contents can be provided via text in \p input_text.
   */
  Parser(const std::vector<std::string> & input_filenames,
         const std::optional<std::vector<std::string>> & input_text = {});
  /**
   * Constructor, given a file in \p input_filename.
   *
   * Optionally, the file contents can be provided via text in \p input_text.
   */
  Parser(const std::string & input_filename, const std::optional<std::string> & input_text = {});

  /**
   * Parses the inputs
   */
  void parse();

  /**
   * This function attempts to extract values from the input file based on the contents of
   * the passed parameters objects.  It handles a number of various types with dynamic casting
   * including vector types
   */
  void extractParams(const std::string & prefix, InputParameters & p);

  /**
   * @return The root HIT node, if any
   *
   * If this is null, it means that we haven't parsed yet
   */
  hit::Node * root() { return _root.get(); }

  /**
   * @return The names of the inputs
   */
  const std::vector<std::string> & getInputFileNames() const { return _input_filenames; }

  /*
   * Get extracted application type from parser
   */
  const std::string & getAppType() const { return _app_type; }

  /*
   * Set the application type in parser
   */
  void setAppType(const std::string & app_type) { _app_type = app_type; }

  /**
   * @return The file name of the last input
   */
  const std::string & getLastInputFileName() const;

  /**
   * @return The path of the last input
   */
  std::filesystem::path getLastInputFilePath() const { return getLastInputFileName(); }

private:
  /// The root node, which owns the whole tree
  std::unique_ptr<hit::Node> _root;

  /// The input file names
  const std::vector<std::string> _input_filenames;

  /// The optional input text contents (to support not reading by file)
  const std::optional<std::vector<std::string>> _input_text;

  /// The application types extracted from [Application] block
  std::string _app_type;
};
