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

  void errorCheck(const libMesh::Parallel::Communicator & comm, bool warn_unused, bool err_unused);

  std::vector<std::string> listValidParams(std::string & section_name);

private:
  /**
   * Helper functions for setting parameters of arbitrary types - bodies are in the .C file
   * since they are called only from this Object
   */
  /// Template method for setting any scalar type parameter read from the input file or command line
  template <typename T, typename Base>
  void setScalarParameter(const std::string & full_name,
                          const std::string & short_name,
                          InputParameters::Parameter<T> * param,
                          bool in_global,
                          GlobalParamsAction * global_block);

  template <typename T, typename UP_T, typename Base>
  void setScalarValueTypeParameter(const std::string & full_name,
                                   const std::string & short_name,
                                   InputParameters::Parameter<T> * param,
                                   bool in_global,
                                   GlobalParamsAction * global_block,
                                   const hit::Node & node);

  /// Template method for setting any vector type parameter read from the input file or command line
  template <typename T, typename Base>
  void setVectorParameter(const std::string & full_name,
                          const std::string & short_name,
                          InputParameters::Parameter<std::vector<T>> * param,
                          bool in_global,
                          GlobalParamsAction * global_block);

  /// Template method for setting any map type parameter read from the input file or command line
  template <typename KeyType, typename MappedType>
  void setMapParameter(const std::string & full_name,
                       const std::string & short_name,
                       InputParameters::Parameter<std::map<KeyType, MappedType>> * param,
                       bool in_global,
                       GlobalParamsAction * global_block);

  /**
   * Template method for setting any double indexed type parameter read from the input file or
   * command line.
   */
  template <typename T>
  void setDoubleIndexParameter(const std::string & full_name,
                               const std::string & short_name,
                               InputParameters::Parameter<std::vector<std::vector<T>>> * param,
                               bool in_global,
                               GlobalParamsAction * global_block);

  /**
   * Template method for setting any triple indexed type parameter read from the input file or
   * command line.
   */
  template <typename T>
  void setTripleIndexParameter(
      const std::string & full_name,
      const std::string & short_name,
      InputParameters::Parameter<std::vector<std::vector<std::vector<T>>>> * param,
      bool in_global,
      GlobalParamsAction * global_block);

  /**
   * Template method for setting any multivalue "scalar" type parameter read from the input file or
   * command line.  Examples include "Point" and "RealVectorValue".
   */
  template <typename T>
  void setScalarComponentParameter(const std::string & full_name,
                                   const std::string & short_name,
                                   InputParameters::Parameter<T> * param,
                                   bool in_global,
                                   GlobalParamsAction * global_block);

  /**
   * Template method for setting several multivalue "scalar" type parameter read from the input
   * file or command line.  Examples include "Point" and "RealVectorValue".
   */
  template <typename T>
  void setVectorComponentParameter(const std::string & full_name,
                                   const std::string & short_name,
                                   InputParameters::Parameter<std::vector<T>> * param,
                                   bool in_global,
                                   GlobalParamsAction * global_block);

  /**
   * Template method for setting vector of several multivalue "scalar" type parameter read from the
   * input file or command line.  Examples include vectors of several "Point"s and
   * "RealVectorValue"s such as (a three-element vector; each element is several "Point"s):
   * points_values = '0 0 0
   *                  0 0 1;
   *                  0 1 0;
   *                  1 0 0
   *                  1 1 0
   *                  1 1 1'
   */
  template <typename T>
  void
  setVectorVectorComponentParameter(const std::string & full_name,
                                    const std::string & short_name,
                                    InputParameters::Parameter<std::vector<std::vector<T>>> * param,
                                    bool in_global,
                                    GlobalParamsAction * global_block);

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

  /// The current parameter object for which parameters are being extracted
  InputParameters * _current_params;

  /// The errors accumulated during the walk
  std::vector<hit::ErrorMessage> _errors;

  /// Deprecation warnings
  std::unordered_map<std::string, std::string> _deprecated_params;

  void walkRaw(std::string fullpath, std::string nodepath, hit::Node * n);
};
}
