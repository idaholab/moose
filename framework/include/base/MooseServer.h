//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseApp.h"
#include "wasplsp/LSP.h"
#include "wasplsp/ServerImpl.h"
#include "wasplsp/Connection.h"
#include "libmesh/ignore_warnings.h"
#include "wasplsp/IOStreamConnection.h"
#include "libmesh/restore_warnings.h"
#include "waspcore/Object.h"
#include "wasphit/HITNodeView.h"
#include "waspsiren/SIRENInterpreter.h"
#include "waspsiren/SIRENResultSet.h"
#include <string>
#include <memory>
#include <set>
#include <map>

class MooseServer : public wasp::lsp::ServerImpl
{
public:
  MooseServer(MooseApp & moose_app);

  virtual ~MooseServer() = default;

  /**
   * Get read / write connection - specific to this server implemention.
   * @return - shared pointer to the server's read / write connection
   */
  std::shared_ptr<wasp::lsp::Connection> getConnection() { return _connection; }

private:
  /**
   * SortedLocationNodes - type alias for set of nodes sorted by location
   */
  using SortedLocationNodes =
      std::set<wasp::HITNodeView,
               std::function<bool(const wasp::HITNodeView &, const wasp::HITNodeView &)>>;

  /**
   * Parse document for diagnostics - specific to this server implemention.
   * @param diagnosticsList - data array of diagnostics data objects to fill
   * @return - true if completed successfully - does not indicate parse fail
   */
  bool parseDocumentForDiagnostics(wasp::DataArray & diagnosticsList);

  /**
   * Update document text changes - specific to this server implemention.
   * @param replacement_text - text to be replaced over the provided range
   * @param start_line - starting replace line number ( zero-based )
   * @param start_character - starting replace column number ( zero-based )
   * @param end_line - ending replace line number ( zero-based )
   * @param end_character - ending replace column number ( zero-based )
   * @param range_length - length of replace range - server specific
   * @return - true if the document text was updated successfully
   */
  bool updateDocumentTextChanges(const std::string & replacement_text,
                                 int start_line,
                                 int start_character,
                                 int end_line,
                                 int end_character,
                                 int range_length);

  /**
   * Gather document completion items - specific to this server implemention.
   * @param completionItems - data array of completion item objects to fill
   * @param is_incomplete - flag indicating if the completions are complete
   * @param line - line to be used for completions gathering logic
   * @param character - column to be used for completions gathering logic
   * @return - true if the gathering of items completed successfully
   */
  bool gatherDocumentCompletionItems(wasp::DataArray & completionItems,
                                     bool & is_incomplete,
                                     int line,
                                     int character);

  /**
   * Get names of parameters and subblocks specified in given input node.
   * @param parent_node - object node context under which to gather input
   * @param existing_params - set to fill with parameter names from input
   * @param existing_subblocks - set to fill with subblock names in input
   */
  void getExistingInput(wasp::HITNodeView parent_node,
                        std::set<std::string> & existing_params,
                        std::set<std::string> & existing_subblocks);

  /**
   * Get all global parameters, action parameters, and object parameters.
   * @param valid_params - collection to fill with valid input parameters
   * @param object_path - full node path where autocomplete was requested
   * @param object_type - type of object where autocomplete was requested
   * @param obj_act_tasks - set for adding in all MooseObjectAction tasks
   */
  void getAllValidParameters(InputParameters & valid_params,
                             const std::string & object_path,
                             const std::string & object_type,
                             std::set<std::string> & obj_act_tasks);

  /**
   * Get all action parameters using requested object path to collection.
   * @param valid_params - collection for filling action input parameters
   * @param object_path - full node path where autocomplete was requested
   * @param obj_act_tasks - set for adding in all MooseObjectAction tasks
   */
  void getActionParameters(InputParameters & valid_params,
                           const std::string & object_path,
                           std::set<std::string> & obj_act_tasks);

  /**
   * Get all object parameters using requested object path to collection.
   * @param valid_params - collection for filling object input parameters
   * @param object_type - type of object where autocomplete was requested
   * @param obj_act_tasks - tasks to verify object type with valid syntax
   */
  void getObjectParameters(InputParameters & valid_params,
                           std::string object_type,
                           const std::set<std::string> & obj_act_tasks);

  /**
   * Add parameters that were previously gathered to list for completion.
   * @param completionItems - list of completion objects to be filled out
   * @param valid_params - all valid parameters to add to completion list
   * @param existing_params - set of parameters already existing in input
   * @param replace_line_beg - start line of autocompletion replace range
   * @param replace_char_beg - start column of autocomplete replace range
   * @param replace_line_end - end line of autocomplete replacement range
   * @param replace_char_end - end column of autocompletion replace range
   * @param filtering_prefix - beginning text to filter list if not empty
   * @return - true if filling of completion items completed successfully
   */
  bool addParametersToList(wasp::DataArray & completionItems,
                           const InputParameters & valid_params,
                           const std::set<std::string> & existing_params,
                           int replace_line_beg,
                           int replace_char_beg,
                           int replace_line_end,
                           int replace_char_end,
                           const std::string & filtering_prefix);

  /**
   * Add subblocks to completion list for request path, line, and column.
   * @param completionItems - list of completion objects to be filled out
   * @param object_path - full node path where autocomplete was requested
   * @param replace_line_beg - start line of autocompletion replace range
   * @param replace_char_beg - start column of autocomplete replace range
   * @param replace_line_end - end line of autocomplete replacement range
   * @param replace_char_end - end column of autocompletion replace range
   * @param filtering_prefix - beginning text to filter list if not empty
   * @return - true if filling of completion items completed successfully
   */
  bool addSubblocksToList(wasp::DataArray & completionItems,
                          const std::string & object_path,
                          int replace_line_beg,
                          int replace_char_beg,
                          int replace_line_end,
                          int replace_char_end,
                          const std::string & filtering_prefix,
                          bool request_on_block_decl);

  /**
   * Add parameter values to completion list for request line and column.
   * @param completionItems - list of completion objects to be filled out
   * @param valid_params - all valid parameters used for value completion
   * @param existing_params - set of parameters already existing in input
   * @param existing_subblocks - active and inactive subblock name values
   * @param param_name - name of input parameter for value autocompletion
   * @param obj_act_tasks - tasks to verify object type with valid syntax
   * @param object_path - full node path where autocomplete was requested
   * @param replace_line_beg - start line of autocompletion replace range
   * @param replace_char_beg - start column of autocomplete replace range
   * @param replace_line_end - end line of autocomplete replacement range
   * @param replace_char_end - end column of autocompletion replace range
   * @return - true if filling of completion items completed successfully
   */
  bool addValuesToList(wasp::DataArray & completionItems,
                       const InputParameters & valid_params,
                       const std::set<std::string> & existing_params,
                       const std::set<std::string> & existing_subblocks,
                       const std::string & param_name,
                       const std::set<std::string> & obj_act_tasks,
                       const std::string & object_path,
                       int replace_line_beg,
                       int replace_char_beg,
                       int replace_line_end,
                       int replace_char_end);

  /**
   * Fill map of all options and descriptions if parameter is moose enum.
   * @param moose_enum_param - parameter to get documentation and options
   * @param options_and_descs - map to fill with options and descriptions
   */
  template <typename MooseEnumType>
  void getEnumsAndDocs(MooseEnumType & moose_enum_param,
                       std::map<std::string, std::string> & options_and_descs);

  /**
   * Gather definition locations - specific to this server implemention.
   * @param definitionLocations - data array of locations objects to fill
   * @param line - line to be used for locations gathering logic
   * @param character - column to be used for locations gathering logic
   * @return - true if the gathering of locations completed successfully
   */
  bool
  gatherDocumentDefinitionLocations(wasp::DataArray & definitionLocations, int line, int character);

  /**
   * Get set of nodes from associated path lookups matching value string.
   * @param location_nodes - set to fill with lookup nodes matching value
   * @param clean_type - cpp type string used for key finding input paths
   * @param val_string - specified value used for gathering input lookups
   */
  void getInputLookupDefinitionNodes(SortedLocationNodes & location_nodes,
                                     const std::string & clean_type,
                                     const std::string & val_string);

  /**
   * Add set of nodes sorted by location to definition or reference list.
   * @param defsOrRefsLocations - data array of locations objects to fill
   * @param location_nodes - set of nodes that have locations to be added
   * @return - true if filling of location objects completed successfully
   */
  bool addLocationNodesToList(wasp::DataArray & defsOrRefsLocations,
                              const SortedLocationNodes & location_nodes);

  /**
   * Get hover display text - logic specific to this server implemention.
   * @param display_text - string reference to add hover text for display
   * @param line - zero-based line to use for finding node and hover text
   * @param character - zero-based column for finding node and hover text
   * @return - true if display text was added or left empty without error
   */
  bool getHoverDisplayText(std::string & display_text, int line, int character);

  /**
   * Gather references locations - specific to this server implemention.
   * @param referencesLocations - data array of locations objects to fill
   * @param line - line to be used for locations gathering logic
   * @param character - column to be used for locations gathering logic
   * @param include_declaration - flag indicating declaration inclusion
   * @return - true if the gathering of locations completed successfully
   */
  bool gatherDocumentReferencesLocations(wasp::DataArray & referencesLocations,
                                         int line,
                                         int character,
                                         bool include_declaration);

  /**
   * Recursively walk input to gather all nodes matching value and types.
   * @param match_nodes - set to fill with nodes matching value and types
   * @param view_parent - nodeview used to start recursive tree traversal
   * @param target_value -
   * @param target_types -
   */
  void getNodesByValueAndTypes(SortedLocationNodes & match_nodes,
                               wasp::HITNodeView view_parent,
                               const std::string & target_value,
                               const std::set<std::string> & target_types);

  /**
   * Gather formatting text edits - specific to this server implemention.
   * @param formattingTextEdits - data array of text edit objects to fill
   * @param tab_size - value of the size of a tab in spaces for formatting
   * @param insert_spaces - flag indicating whether to use spaces for tabs
   * @return - true if the gathering of text edits completed successfully
   */
  bool gatherDocumentFormattingTextEdits(wasp::DataArray & formattingTextEdits,
                                         int tab_size,
                                         bool insert_spaces);

  /**
   * Recursively walk down whole nodeview tree while formatting document.
   * @param parent - nodeview for recursive tree traversal starting point
   * @param prev_line - line of last print for blanks and inline comments
   * @param level - current level in document tree to use for indentation
   * @return - formatted string that gets appended to each recursive call
   */
  std::string formatDocument(wasp::HITNodeView parent, std::size_t & prev_line, std::size_t level);

  /**
   * Gather document symbols - specific to this server implemention.
   * @param documentSymbols - data array of symbols data objects to fill
   * @return - true if the gathering of symbols completed successfully
   */
  bool gatherDocumentSymbols(wasp::DataArray & documentSymbols);

  /**
   * Recursively fill document symbols from the given node.
   * @param view_parent - nodeview used in recursive tree traversal
   * @param data_parent - data object with array of symbol children
   * @return - true if no problems with this level of the resursion
   */
  bool traverseParseTreeAndFillSymbols(wasp::HITNodeView view_parent,
                                       wasp::DataObject & data_parent);

  /**
   * Get completion item kind value that client may use for icon in list.
   * @param valid_params - valid parameters used for completion item kind
   * @param param_name - name of input parameter for completion item kind
   * @param clean_type - type to decide if reference completion item kind
   * @param is_param - boolean denoting if kind is for parameter or value
   * @return - enumerated kind value that client may use for icon in list
   */
  int getCompletionItemKind(const InputParameters & valid_params,
                            const std::string & param_name,
                            const std::string & clean_type,
                            bool is_param);

  /**
   * Get document symbol kind value that client may use for outline icon.
   * @param symbol_node - node that will be added to symbol tree for kind
   * @return - enumerated kind value that client may use for outline icon
   */
  int getDocumentSymbolKind(wasp::HITNodeView symbol_node);

  /**
   * Get required parameter completion text list for given subblock path.
   * @param subblock_path - subblock path for finding required parameters
   * @param subblock_type - subblock type for finding required parameters
   * @param existing_params - set of parameters already existing in input
   * @param indent_spaces - indentation to be added before each parameter
   * @return - list of required parameters to use in subblock insert text
   */
  std::string getRequiredParamsText(const std::string & subblock_path,
                                    const std::string & subblock_type,
                                    const std::set<std::string> & existing_params,
                                    const std::string & indent_spaces);

  /**
   * Read from connection into object - specific to this server's connection.
   * @param object - reference to object to be read into
   * @return - true if the read from the connection completed successfully
   */
  bool connectionRead(wasp::DataObject & object) { return _connection->read(object, errors); }

  /**
   * Write object json to connection - specific to this server's connection.
   * @param object - reference to object with contents to write to connection
   * @return - true if the write to the connection completed successfully
   */
  bool connectionWrite(wasp::DataObject & object) { return _connection->write(object, errors); }

  /**
   * @return Whether or not the root is valid
   *
   * Will be true if the app is valid, the root is not nullptr, and the root node view is not null
   */
  bool rootIsValid() const;

  /**
   * @return The current root node
   */
  hit::Node & getRoot();

  /**
   * @return Input check application for document path from current operation
   */
  std::shared_ptr<MooseApp> getCheckApp() const;

  /**
   * @return up to date text string associated with current document path
   */
  const std::string & getDocumentText() const;

  /**
   * @brief _moose_app - reference to parent application that owns this server
   */
  MooseApp & _moose_app;

  /**
   * @brief _check_apps - map from document paths to input check applications
   */
  std::map<std::string, std::shared_ptr<MooseApp>> _check_apps;

  /**
   * @brief _path_to_text - map of document paths to current text strings
   */
  std::map<std::string, std::string> _path_to_text;

  /**
   * @brief _connection - shared pointer to this server's read / write iostream
   */
  std::shared_ptr<wasp::lsp::IOStreamConnection> _connection;

  /**
   * @brief _syntax_to_subblocks - map of syntax paths to valid subblocks
   */
  std::map<std::string, std::set<std::string>> _syntax_to_subblocks;

  /**
   * @brief _type_to_input_paths - map of parameter types to lookup paths
   */
  std::map<std::string, std::set<std::string>> _type_to_input_paths;

  /**
   * @brief _type_to_input_paths - map of lookup paths to parameter types
   */
  std::map<std::string, std::set<std::string>> _input_path_to_types;

  /**
   * @brief _formatting_tab_size - number of indent spaces for formatting
   */
  std::size_t _formatting_tab_size;
};
