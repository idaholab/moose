//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "nlohmann/json.h"
#include <string>
#include <vector>
#include <utility>
#include <optional>

#include "libmesh/parameters.h"

class Syntax;
class ActionFactory;
class Factory;
class FileLineInfo;

/**
 * Builder for the syntax tree in JSON
 */
class JsonSyntaxTree
{
public:
  JsonSyntaxTree(const Syntax & syntax,
                 const ActionFactory & action_factory,
                 const Factory & factory,
                 const std::optional<std::string> & search = {});

  virtual ~JsonSyntaxTree() {}

  /**
   * Builds the JSON syntax tree.
   */
  nlohmann::json build();

  /**
   * Converts a c++ type into a "basic" type
   */
  static std::string basicCppType(const std::string & cpp_type);

  /**
   *
   */
  static std::string
  buildOutputString(const std::iterator_traits<InputParameters::const_iterator>::value_type & p);

  static std::string
  buildOptions(const std::iterator_traits<InputParameters::const_iterator>::value_type & p,
               bool & out_of_range_allowed,
               std::map<MooseEnumItem, std::string> & docs);

protected:
  /**
   * Add parameters to the tree
   * @param parent_path The parent syntax path that the action belongs to
   * @param path The path of the action
   * @param is_type Whether this belongs to a "<type>" or not
   * @param action Name of the action
   * @param is_action Whether we are adding the parameter for an action (except Components)
   * @param params The InputParameters to add to the tree
   * @param lineinfo The FileLineInfo where the action/path was registered
   * @param classname the name of the class being added
   * @return Whether the parameters were added to the tree (ie if it matched the search string).
   */
  bool addParameters(const std::string & parent_path,
                     const std::string & path,
                     bool is_type,
                     const std::string & action,
                     bool is_action,
                     const InputParameters & params,
                     const FileLineInfo & lineinfo,
                     const std::string & classname);

  /**
   * Add the global section to the output
   */
  void addGlobal();

  /**
   * Insert all of the parameters into the tree
   * @param params The parameters to read from
   * @param search_match Whether or not the search in the tree above was a match
   * @param all_params The JSON param node to fill into
   * @return The number of parameters that were set
   */
  size_t
  setParams(const InputParameters & params, const bool search_match, nlohmann::json & all_params);

  /**
   * Gets the entry in the JSON tree for the given path
   * @param parent The path to the parent
   * @param path The path to the object
   * @param is_type Whether or not the params are for a type (if false, an action)
   * @return The JSON node
   */
  nlohmann::json &
  getJson(const std::string & parent, const std::string & path, const bool is_type);
  /**
   * Gets the path in the JSON tree
   * @param path The path
   * @return The JSON node
   */
  nlohmann::json & getJson(const std::string & path);

  /**
   * Get the cached label (label and file) for the given object path \p obj
   */
  const std::pair<std::string, std::string> & getObjectLabel(const std::string & obj) const;
  /**
   * Get the cached label (label and file) for the given action with name \p action
   */
  const std::pair<std::string, std::string> & getActionLabel(const std::string & action) const;

  /// The root JSON node for syntax; filled during build() and used within helper functions
  std::unique_ptr<nlohmann::json> _root;
  /// The application's Syntax
  const Syntax & _syntax;
  /// The application' ActionFactory
  const ActionFactory & _action_factory;
  /// The application's Factory
  const Factory & _factory;
  /// Optional search parameter
  const std::optional<std::string> _search;

  ///@{
  /// Maps storing action/object name to the label and file location
  std::map<std::string, std::pair<std::string, std::string>> _action_label_map;
  std::map<std::string, std::pair<std::string, std::string>> _object_label_map;
  ///@}
};
