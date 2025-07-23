//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

#include "hit/hit.h"

#include <vector>
#include <string>
#include <iomanip>
#include <optional>
#include <filesystem>

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

  std::vector<hit::ErrorMessage> errors;

private:
  std::map<std::string, hit::Node *> _have;
};

class BadActiveWalker : public hit::Walker
{
public:
  virtual void walk(const std::string & /*fullpath*/,
                    const std::string & /*nodepath*/,
                    hit::Node * section) override;
  std::vector<hit::ErrorMessage> errors;
};

class CompileParamWalker : public hit::Walker
{
public:
  typedef std::map<std::string, hit::Node *> ParamMap;
  CompileParamWalker(ParamMap & map) : _map(map) {};

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

  struct Error : public hit::Error
  {
    Error() = delete;
    Error(const std::vector<hit::ErrorMessage> & error_messages);
  };

  /**
   * Parses the inputs
   */
  void parse();

  /**
   * @return The root HIT node if it exists
   *
   * If this is null, it means we haven't parsed yet
   */
  ///@{
  const hit::Node * queryRoot() const { return _root.get(); }
  hit::Node * queryRoot() { return _root.get(); }
  ///@}

  /**
   * @return The root HIT node with error checking on if it exists
   *
   * If it doesn't exist, it means we haven't parsed yet
   */
  hit::Node & getRoot();

  /**
   * @return The root command line HIT node if it exists
   *
   * If this is null, it means we haven't parsed yet
   */
  ///@{
  const hit::Node * queryCommandLineRoot() const { return _cli_root.get(); }
  hit::Node * queryCommandLineRoot() { return _cli_root.get(); }
  ///@}

  /**
   * @return The root command line HIT node, with error checking on if it exists
   *
   * If it doesn't exist, it means we haven't parsed yet
   */
  ///@{
  const hit::Node & getCommandLineRoot() const;
  hit::Node & getCommandLineRoot();
  ///@}

  /**
   * @return The names of the inputs
   */
  const std::vector<std::string> & getInputFileNames() const { return _input_filenames; }

  /**
   * @return The input file contents
   */
  const std::vector<std::string> & getInputText() const { return _input_text; }

  /*
   * Get extracted application type from parser
   */
  const std::string & getAppType() const { return _app_type; }

  /*
   * Set the application type in parser
   */
  void setAppType(const std::string & app_type) { _app_type = app_type; }

  /**
   * Sets the HIT parameters from the command line
   */
  void setCommandLineParams(const std::vector<std::string> & params);

  /**
   * @return The file name of the last input
   */
  const std::string & getLastInputFileName() const;

  /**
   * @return The path of the last input
   */
  std::filesystem::path getLastInputFilePath() const { return getLastInputFileName(); }

  /**
   * Set whether or not to throw Parse::Error on errors
   *
   * This is used by the MooseServer to capture errors while retaining the root if possible
   */
  void setThrowOnError(const bool throw_on_error) { _throw_on_error = throw_on_error; }

  /**
   * @return Whether or not to throw Parse::Error on errors
   *
   * This is used by the MooseServer to capture errors while retaining the root if possible
   */
  bool getThrowOnError() const { return _throw_on_error; }

  /**
   * @returns The variables that have been extracted so far.
   *
   * These are the variables that have been used during brace expansion.
   */
  const std::set<std::string> & getExtractedVars() const { return _extracted_vars; }
  /**
   * Helper for accumulating errors from a walker into an accumulation of errors
   */
  ///@{
  static void appendErrorMessages(std::vector<hit::ErrorMessage> & to,
                                  const std::vector<hit::ErrorMessage> & from);
  static void appendErrorMessages(std::vector<hit::ErrorMessage> & to, const hit::Error & error);
  ///@}

  /**
   * Helper for combining error messages into a single, newline separated message
   */
  static std::string joinErrorMessages(const std::vector<hit::ErrorMessage> & error_messages);

  /**
   * Helper for throwing an error with the given messages.
   *
   * If throwOnError(), throw a Parser::Error (for the MooseServer).
   * Otherwise, use mooseError() (for standard runs).
   */
  void parseError(std::vector<hit::ErrorMessage> messages) const;

private:
  /// The root node, which owns the whole tree
  std::unique_ptr<hit::Node> _root;

  /// The input file names
  const std::vector<std::string> _input_filenames;

  /// The input text (may be filled during parse())
  std::vector<std::string> _input_text;

  /// The root node for command line hit arguments
  std::unique_ptr<hit::Node> _cli_root;

  /// The application types extracted from [Application] block
  std::string _app_type;

  /// Whether or not to throw on error
  bool _throw_on_error;

  /// The command line HIT parameters (if any)
  std::optional<std::vector<std::string>> _command_line_params;

  /// Variables that have been extracted during brace expansion
  std::set<std::string> _extracted_vars;
};
