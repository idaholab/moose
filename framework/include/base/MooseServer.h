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

#include <string>
#include <memory>
#include "wasplsp/LSP.h"
#include "wasplsp/ServerImpl.h"
#include "wasplsp/Connection.h"
#include "wasplsp/IOStreamConnection.h"
#include "waspcore/Object.h"
#include "wasphit/HITNodeView.h"
#include "MooseApp.h"

class MooseServer : public wasp::lsp::ServerImpl
{
public:
  MooseServer(MooseApp & moose_app);

  virtual ~MooseServer() = default;

  /** get read / write connection - specific to this server implemention
   * @return - shared pointer to the server's read / write connection
   */
  std::shared_ptr<wasp::lsp::Connection> getConnection() { return _connection; }

private:
  /** parse document for diagnostics - specific to this server implemention
   * @param diagnosticsList - data array of diagnostics data objects to fill
   * @return - true if completed successfully - does not indicate parse fail
   */
  bool parseDocumentForDiagnostics(wasp::DataArray & diagnosticsList);

  /** update document text changes - specific to this server implemention
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

  /** gather document completion items - specific to this server implemention
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

  /** gather definition locations - specific to this server implemention
   * @param definitionLocations - data array of locations objects to fill
   * @param line - line to be used for locations gathering logic
   * @param character - column to be used for locations gathering logic
   * @return - true if the gathering of locations completed successfully
   */
  bool
  gatherDocumentDefinitionLocations(wasp::DataArray & definitionLocations, int line, int character);

  /** gather references locations - specific to this server implemention
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

  /** gather formatting text edits - specific to this server implemention
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

  /** gather document symbols - specific to this server implemention
   * @param documentSymbols - data array of symbols data objects to fill
   * @return - true if the gathering of symbols completed successfully
   */
  bool gatherDocumentSymbols(wasp::DataArray & documentSymbols);

  /** recursively fill document symbols from the given node
   * @param view_parent - nodeview used in recursive tree traversal
   * @param data_parent - data object with array of symbol children
   * @return - true if no problems with this level of the resursion
   */
  bool traverseParseTreeAndFillSymbols(wasp::HITNodeView view_parent,
                                       wasp::DataObject & data_parent);

  /** read from connection into object - specific to this server's connection
   * @param object - reference to object to be read into
   * @return - true if the read from the connection completed successfully
   */
  bool connectionRead(wasp::DataObject & object) { return _connection->read(object, errors); }

  /** write object json to connection - specific to this server's connection
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
};

#endif
