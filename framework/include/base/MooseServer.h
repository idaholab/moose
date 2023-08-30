//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef WASP_ENABLED

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
   * @param valid_params - collection for filling action input parameters
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
   * @param request_line - line in input where autocomplete was requested
   * @param request_char - char in input where autocomplete was requested
   */
  bool addParametersToList(wasp::DataArray & completionItems,
                           const InputParameters & valid_params,
                           const std::set<std::string> & existing_params,
                           int request_line,
                           int request_char);

  /**
   * Add subblocks to completion list for request path, line, and column.
   * @param completionItems - list of completion objects to be filled out
   * @param object_path - full node path where autocomplete was requested
   * @param request_line - line in input where autocomplete was requested
   * @param request_char - char in input where autocomplete was requested
   * @return - true if filling of completion items completed successfully
   */
  bool addSubblocksToList(wasp::DataArray & completionItems,
                          const std::string & object_path,
                          int request_line,
                          int request_char);

  /**
   * Add parameter values to completion list for request line and column.
   * @param completionItems - list of completion objects to be filled out
   * @param valid_params - all valid parameters used for value completion
   * @param existing_subblocks - active and inactive subblock name values
   * @param param_name - name of input parameter for value autocompletion
   * @param obj_act_tasks - tasks to verify object type with valid syntax
   * @param replace_line_start - start line of autocomplete replace range
   * @param replace_char_start - start char of autocomplete replace range
   * @param replace_line_end - end line of autocomplete replacement range
   * @param replace_char_end - end char of autocomplete replacement range
   * @return - true if filling of completion items completed successfully
   */
  bool addValuesToList(wasp::DataArray & completionItems,
                       const InputParameters & valid_params,
                       const std::set<std::string> & existing_subblocks,
                       const std::string & param_name,
                       const std::set<std::string> & obj_act_tasks,
                       int replace_line_start,
                       int replace_char_start,
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
   * Gather formatting text edits - specific to this server implemention.
   * @param formattingTextEdits - data array of text edit objects to fill
   * @param start_line - starting line to be used for formatting logic
   * @param start_character - starting column to be used for formatting logic
   * @param end_line - ending line to be used for formatting logic
   * @param end_character - ending column to be used for formatting logic
   * @param tab_size - value of the size of a tab in spaces for formatting
   * @param insert_spaces - flag indicating whether to use spaces for tabs
   * @return - true if the gathering of text edits completed successfully
   */
  bool gatherDocumentFormattingTextEdits(wasp::DataArray & formattingTextEdits,
                                         int start_line,
                                         int start_character,
                                         int end_line,
                                         int end_character,
                                         int tab_size,
                                         bool insert_spaces);

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
   * @brief _moose_app - reference to parent application that owns this server
   */
  MooseApp & _moose_app;

  /**
   * @brief _check_app - application created to check input and access parser
   */
  std::shared_ptr<MooseApp> _check_app;

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
};

#endif
