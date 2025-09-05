//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "InputParameters.h"

#include "hit/hit.h"

#include <set>
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
class JsonSyntaxTree;
class Parser;
class Syntax;

namespace Moose
{
class Builder;

class UnusedWalker : public hit::Walker
{
public:
  UnusedWalker(std::set<std::string> used, Builder & p) : _used(used), _builder(p) {}

  void walk(const std::string & fullpath, const std::string & nodename, hit::Node * n) override;

  std::vector<hit::ErrorMessage> errors;

private:
  std::set<std::string> _used;
  Builder & _builder;
};

/**
 * Parses MOOSE input using HIT/WASP.
 */
class Builder : public ConsoleStreamInterface, public hit::Walker
{
public:
  enum SyntaxFormatterType
  {
    INPUT_FILE,
    YAML
  };

  Builder(MooseApp & app, ActionWarehouse & action_wh, Parser & parser);
  virtual ~Builder();

  /**
   * Parameters that are processed directly by the Parser and are valid anywhere in the input
   */
  static InputParameters validParams();

  /**
   * Return the primary (first) filename that was parsed
   */
  std::string getPrimaryFileName(bool stripLeadingPath = true) const;

  /**
   * Parse an input file (or text string if provided) consisting of hit syntax and setup objects
   * in the MOOSE derived application
   */
  void build();

  /**
   * Attempt to extract values from input starting with the section in input in \p section_node
   * based on the contents of the passed InputParameters \p p.
   *
   * If \p section_node is not provided, only the global parameters will be checked
   */
  void extractParams(const hit::Node * const section_node, InputParameters & p);
  /**
   * Attempt to extract values from input starting with the section in input defined
   * by the fullpath \p prefix based on the contents of the passed InputParameters \p p.
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

  void errorCheck(const libMesh::Parallel::Communicator & comm, bool warn_unused, bool err_unused);

  std::vector<std::string> listValidParams(std::string & section_name);

private:
  /**
   * @return Whether or not the given node \p node exists within the [GlobalParams] block
   */
  bool isGlobal(const hit::Node & node) const;

  /**
   * Get the [GlobalParams] section node if it exists
   *
   * We need to separate this so that we can call extractParams()
   * before calling build()
   */
  const hit::Node * queryGlobalParamsNode() const;

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
  /// The front parser
  Parser & _parser;
  /// The root node from the Parser
  hit::Node & _root;

  /// Object for holding the syntax parse tree
  std::unique_ptr<SyntaxTree> _syntax_formatter;

  /// The set of all variables extracted from the input file
  std::set<std::string> _extracted_vars;

  /// The sections that we need to execute first (read during the final walk)
  std::vector<std::string> _secs_need_first;

  /// The errors accumulated during the walk
  std::vector<hit::ErrorMessage> _errors;

  /// Deprecation warnings (object type/param name) -> (message)
  std::map<std::string, std::string> _deprecated_params;

  /// The hit Node for the [GlobalParams] block, if any
  /// If set (could be null), it means we have searched for it
  mutable std::optional<const hit::Node *> _global_params_node;

  void walkRaw(std::string fullpath, std::string nodepath, hit::Node * n);
};
}
