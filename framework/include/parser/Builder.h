//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BuilderBase.h"

// MOOSE includes
#include "ConsoleStreamInterface.h"
#include "MooseTypes.h"
#include "Syntax.h"
#include "AppBuilder.h"

// Forward declarations
class ActionWarehouse;
class SyntaxTree;
class MooseApp;
class Factory;
class ActionFactory;
class JsonSyntaxTree;

namespace Moose
{
class Builder;

class UnusedWalker : public hit::Walker
{
public:
  UnusedWalker(const std::set<std::string> & used, Builder & p) : _used(used), _builder(p) {}

  void walk(const std::string & fullpath, const std::string & nodename, hit::Node * n) override;

  std::vector<std::string> errors;

private:
  const std::set<std::string> & _used;
  Builder & _builder;
};

/**
 * Parses MOOSE input using HIT/WASP.
 */
class Builder : public BuilderBase, public ConsoleStreamInterface, public hit::Walker
{
public:
  enum SyntaxFormatterType
  {
    INPUT_FILE,
    YAML
  };

  Builder(MooseApp & app,
          ActionWarehouse & action_wh,
          std::shared_ptr<Parser> parser,
          const AppBuilder::State & app_builder_state);
  virtual ~Builder();

  /**
   * Parameters that are processed directly by the Parser and are valid anywhere in the input
   */
  static InputParameters validParams();

  /**
   * Parse an input file (or text string if provided) consisting of hit syntax and setup objects
   * in the MOOSE derived application
   */
  void build();

  /**
   * This function attempts to extract values from the input file based on the contents of
   * the passed parameters objects.  It handles a number of various types with dynamic casting
   * including vector types
   */
  void extractParams(const std::string & prefix, InputParameters & p);

  /**
   * Creates a syntax formatter for printing
   */
  void initSyntaxFormatter(SyntaxFormatterType type, bool dump_mode);

  /**
   * Use MOOSE Factories to construct a full parse tree for documentation or echoing input.
   */
  void buildFullTree(const std::string & search_string);

  /**
   * Use MOOSE Factories to construct a parameter tree for documentation or echoing input.
   */
  void buildJsonSyntaxTree(JsonSyntaxTree & tree) const;

  void walk(const std::string & fullpath, const std::string & nodepath, hit::Node * n);

  void errorCheck(const Parallel::Communicator & comm, bool warn_unused, bool err_unused);

  std::vector<std::string> listValidParams(const std::string & section_name);

protected:
  std::vector<std::string> _secs_need_first;

  /// The MooseApp this Parser is part of
  MooseApp & _app;
  /// The Factory associated with that MooseApp
  Factory & _factory;
  /// Action warehouse that will be filled by actions
  ActionWarehouse & _action_wh;
  /// The Factory that builds actions
  ActionFactory & _action_factory;
  /// Reference to an object that defines input file syntax
  Syntax & _syntax;
  const AppBuilder::State & _app_builder_state;

  /// Object for holding the syntax parse tree
  std::unique_ptr<SyntaxTree> _syntax_formatter;

  /// Boolean to indicate whether parsing has started (sections have been extracted)
  bool _sections_read;

private:
  void walkRaw(std::string fullpath, std::string nodepath, hit::Node * n);
};
}
