//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest_include.h"

#include "MooseServer.h"
#include "MooseApp.h"
#include "Moose.h"
#include "MooseMain.h"
#include "AppFactory.h"
#include "MooseUtils.h"
#include "pcrecpp.h"
#include "waspcore/Object.h"
#include "wasplsp/LSP.h"
#include "wasplsp/SymbolIterator.h"
#include <string>
#include <sstream>
#include <memory>
#include <vector>
#include <fstream>
#include <cstdio>

class MooseServerTest : public ::testing::Test
{
protected:
  // traverse diagnostics and make formatted list for easy testing and viewing
  void format_diagnostics(const wasp::DataArray & diagnostics_array,
                          std::ostringstream & diagnostics_stream) const
  {
    std::size_t diagnostics_size = diagnostics_array.size();

    for (std::size_t i = 0; i < diagnostics_size; i++)
    {
      std::stringstream diagnostic_errors;
      int diagnostic_start_line;
      int diagnostic_start_character;
      int diagnostic_end_line;
      int diagnostic_end_character;
      int diagnostic_severity;
      std::string diagnostic_code;
      std::string diagnostic_source;
      std::string diagnostic_message;

      EXPECT_TRUE(wasp::lsp::dissectDiagnosticObject(*(diagnostics_array.at(i).to_object()),
                                                     diagnostic_errors,
                                                     diagnostic_start_line,
                                                     diagnostic_start_character,
                                                     diagnostic_end_line,
                                                     diagnostic_end_character,
                                                     diagnostic_severity,
                                                     diagnostic_code,
                                                     diagnostic_source,
                                                     diagnostic_message));

      // trim any ending whitespace off message which is guaranteed non-empty
      diagnostic_message.erase(diagnostic_message.find_last_not_of(" \t") + 1);

      diagnostics_stream << "line:" << diagnostic_start_line
                         << " column:" << diagnostic_start_character << " - " << diagnostic_message
                         << "\n";
    }
  }

  // traverse symbolpaths and make formatted list for easy testing and viewing
  void format_symbolpaths(const wasp::DataObject & symbols_response,
                          std::ostringstream & paths_stream) const
  {
    EXPECT_TRUE(wasp::lsp::verifySymbolsResponse(symbols_response));
    wasp::lsp::SymbolIterator si(std::make_shared<wasp::DataObject>(symbols_response));

    for (std::vector<int> indices{0}; indices.back() < (int)si.getChildSize(); indices.back()++)
    {
      si.moveToChildAt(indices.back());
      indices.push_back(-1);

      std::string symbol_name;
      std::string symbol_detail;
      int symbol_kind;
      bool symbol_deprecated;
      int symbol_start_line;
      int symbol_start_character;
      int symbol_end_line;
      int symbol_end_character;
      int symbol_selection_start_line;
      int symbol_selection_start_character;
      int symbol_selection_end_line;
      int symbol_selection_end_character;

      EXPECT_TRUE(si.dissectCurrentSymbol(symbol_name,
                                          symbol_detail,
                                          symbol_kind,
                                          symbol_deprecated,
                                          symbol_start_line,
                                          symbol_start_character,
                                          symbol_end_line,
                                          symbol_end_character,
                                          symbol_selection_start_line,
                                          symbol_selection_start_character,
                                          symbol_selection_end_line,
                                          symbol_selection_end_character));

      paths_stream << std::setw(34) << std::left << si.getPath() << " detail: " << std::setw(13)
                   << std::left << symbol_detail << " kind: " << std::setw(2) << std::right
                   << symbol_kind << "\n";

      if (si.getChildSize() == 0)
      {
        while (indices.back() + 1 == (int)si.getChildSize() && si.moveToParent())
        {
          indices.pop_back();
        }
      }
    }
  }

  // traverse completions and make formatted list for easy testing and viewing
  void format_completions(const wasp::DataArray & completions_array,
                          std::ostringstream & completions_stream) const
  {
    std::size_t completions_size = completions_array.size();

    for (std::size_t i = 0; i < completions_size; i++)
    {
      std::stringstream completion_errors;
      std::string completion_label;
      int completion_start_line;
      int completion_start_character;
      int completion_end_line;
      int completion_end_character;
      std::string completion_new_text;
      int completion_kind;
      std::string completion_detail;
      std::string completion_documentation;
      bool completion_deprecated;
      bool completion_preselect;
      int completion_text_format;

      EXPECT_TRUE(wasp::lsp::dissectCompletionObject(*(completions_array.at(i).to_object()),
                                                     completion_errors,
                                                     completion_label,
                                                     completion_start_line,
                                                     completion_start_character,
                                                     completion_end_line,
                                                     completion_end_character,
                                                     completion_new_text,
                                                     completion_kind,
                                                     completion_detail,
                                                     completion_documentation,
                                                     completion_deprecated,
                                                     completion_preselect,
                                                     completion_text_format));

      // truncate long descriptions and escape text newlines for easy viewing
      std::size_t description_truncate_length = 20;
      if (completion_documentation.length() > description_truncate_length)
      {
        completion_documentation.resize(description_truncate_length - 3);
        completion_documentation += "...";
      }

      MooseUtils::escape(completion_new_text);

      // transform completion text format to string representation for test
      std::string text_format_string = "invalid";
      if (completion_text_format == wasp::lsp::m_text_format_plaintext)
        text_format_string = "regular";
      else if (completion_text_format == wasp::lsp::m_text_format_snippet)
        text_format_string = "snippet";

      std::ostringstream completion_object;
      completion_object << "label: " << completion_label << " text: " << completion_new_text
                        << " desc: " << completion_documentation << " pos: ["
                        << completion_start_line << "." << completion_start_character << "]-["
                        << completion_end_line << "." << completion_end_character
                        << "] kind: " << completion_kind << " format: " << text_format_string;
      completions_stream << MooseUtils::removeExtraWhitespace(completion_object.str()) << "\n";
    }
  }

  // traverse locations and make formatted list for easy testing and viewing
  void format_locations(const wasp::DataArray & locations_array,
                        std::ostringstream & locations_stream) const
  {
    std::string uri_pattern = "(" + std::string(wasp::lsp::m_uri_prefix) + ")(/.*/framework/)(.*)";
    std::string uri_replace = "\\1...absolute.../framework/\\3";

    std::size_t locations_size = locations_array.size();

    for (std::size_t i = 0; i < locations_size; i++)
    {
      std::stringstream location_errors;
      std::string location_uri;
      int location_start_line;
      int location_start_character;
      int location_end_line;
      int location_end_character;

      EXPECT_TRUE(wasp::lsp::dissectLocationObject(*(locations_array.at(i).to_object()),
                                                   location_errors,
                                                   location_uri,
                                                   location_start_line,
                                                   location_start_character,
                                                   location_end_line,
                                                   location_end_character));

      // remove machine specific parts from absolute path for source code uri

      pcrecpp::RE(uri_pattern).Replace(uri_replace, &location_uri);

      locations_stream << "document_uri: \"" << location_uri << "\""
                       << "    location_start: [" << location_start_line << "."
                       << location_start_character << "]    location_end: [" << location_end_line
                       << "." << location_end_character << "]"
                       << "\n";
    }
  }

  // traverse textedits and make formatted list for easy testing and viewing
  void format_textedits(const wasp::DataArray & textedits_array,
                        std::ostringstream & textedits_stream) const
  {
    std::size_t textedits_size = textedits_array.size();

    for (std::size_t i = 0; i < textedits_size; i++)
    {
      std::stringstream textedit_errors;
      int textedit_beg_line;
      int textedit_beg_char;
      int textedit_end_line;
      int textedit_end_char;
      std::string textedit_new_text;

      EXPECT_TRUE(wasp::lsp::dissectTextEditObject(*(textedits_array.at(i).to_object()),
                                                   textedit_errors,
                                                   textedit_beg_line,
                                                   textedit_beg_char,
                                                   textedit_end_line,
                                                   textedit_end_char,
                                                   textedit_new_text));

      textedits_stream << "textedit_position:"
                       << " [" << textedit_beg_line << "." << textedit_beg_char << "]"
                       << "-[" << textedit_end_line << "." << textedit_end_char << "]\n"
                       << "textedit_new_text:\n"
                       << textedit_new_text << "\n";
    }
  }

  // build completion request, handle request with server, and check response
  void check_completions(int req_id,
                         const std::string & req_uri,
                         int req_line,
                         int req_char,
                         std::size_t expect_count,
                         const std::string & expect_items) const
  {
    // build completion request with the test parameters
    wasp::DataObject completion_request;
    std::stringstream completion_errors;
    EXPECT_TRUE(wasp::lsp::buildCompletionRequest(
        completion_request, completion_errors, req_id, req_uri, req_line, req_char));
    EXPECT_TRUE(completion_errors.str().empty());

    // handle the built completion request with the moose_server
    wasp::DataObject completion_response;
    EXPECT_TRUE(moose_server->handleCompletionRequest(completion_request, completion_response));
    EXPECT_TRUE(moose_server->getErrors().empty());

    // check the dissected values of the moose_server completion response
    std::stringstream response_errors;
    int response_id;
    bool response_is_incomplete;
    wasp::DataArray completions_array;
    EXPECT_TRUE(wasp::lsp::dissectCompletionResponse(completion_response,
                                                     response_errors,
                                                     response_id,
                                                     response_is_incomplete,
                                                     completions_array));
    EXPECT_TRUE(response_errors.str().empty());
    EXPECT_EQ(req_id, response_id);

    // check greater than or equal to allow syntax be added without failing
    EXPECT_GE(completions_array.size(), expect_count);

    // make formatted list from completion items and check it is as expected
    std::ostringstream actual_items;
    format_completions(completions_array, actual_items);

    // collapse repeated spaces to remove column format from expected items
    auto expect_collapsed = MooseUtils::removeExtraWhitespace(expect_items);

    // check that each line exists to allow syntax be added without failing
    for (const auto & line : MooseUtils::split(expect_collapsed, "label:"))
      EXPECT_NE(actual_items.str().find(line), std::string::npos)
          << "did not find: \"" << line << "\"";
  }

  // build hover request, handle request with moose_server, check response
  void check_hover(int request_id,
                   const std::string & request_uri,
                   int request_line,
                   int request_char,
                   const std::string & expect_text) const
  {
    // build the request with the provided parameters for the moose_server
    wasp::DataObject hover_request;
    std::stringstream hover_errors;
    EXPECT_TRUE(wasp::lsp::buildHoverRequest(
        hover_request, hover_errors, request_id, request_uri, request_line, request_char));
    EXPECT_TRUE(hover_errors.str().empty());

    // handle the request built from the parameters using the moose_server
    wasp::DataObject hover_response;
    EXPECT_TRUE(moose_server->handleHoverRequest(hover_request, hover_response));
    EXPECT_TRUE(moose_server->getErrors().empty());

    // check the dissected values of the response sent by the moose_server
    std::stringstream response_errors;
    int response_id;
    std::string actual_text;
    EXPECT_TRUE(
        wasp::lsp::dissectHoverResponse(hover_response, response_errors, response_id, actual_text));
    EXPECT_TRUE(response_errors.str().empty());
    EXPECT_EQ(request_id, response_id);
    EXPECT_EQ(expect_text, actual_text);
  }

  // create moose_unit_app and moose_server to persist for reuse between tests
  static void SetUpTestCase()
  {
    moose_unit_app = Moose::createMooseApp("MooseUnitApp", 0, nullptr);

    moose_server = std::make_unique<MooseServer>(*moose_unit_app);
  }

  // delete moose_unit_app and moose_server after all test runs have completed
  static void TearDownTestCase()
  {
    moose_unit_app.reset();
    moose_server.reset();
  }

  // statically declare moose_unit_app and moose_server for reuse across tests
  static std::shared_ptr<MooseApp> moose_unit_app;
  static std::unique_ptr<MooseServer> moose_server;
};

// define moose_unit_app and moose_server that are declared as static in class
std::shared_ptr<MooseApp> MooseServerTest::moose_unit_app;
std::unique_ptr<MooseServer> MooseServerTest::moose_server;

TEST_F(MooseServerTest, InitializeAndInitialized)
{
  // initialize test parameters

  int request_id = 1;
  int process_id = -1;
  std::string root_path = "";

  // enable client snippet capability so server uses that completion syntax
  wasp::DataObject client_caps, textdoc_caps, complete_caps, compitem_caps;
  compitem_caps[wasp::lsp::m_snip] = true;
  complete_caps[wasp::lsp::m_compitem] = compitem_caps;
  textdoc_caps[wasp::lsp::m_comp] = complete_caps;
  client_caps[wasp::lsp::m_text_document] = textdoc_caps;

  // build initialize request with the test parameters

  wasp::DataObject initialize_request;
  std::stringstream initialize_errors;

  EXPECT_TRUE(wasp::lsp::buildInitializeRequest(
      initialize_request, initialize_errors, request_id, process_id, root_path, client_caps));

  EXPECT_TRUE(initialize_errors.str().empty());

  // handle the built initialize request with the moose_server

  wasp::DataObject initialize_response;

  // check snippet support is disabled by default before initialize request
  EXPECT_FALSE(moose_server->clientSupportsSnippets());

  EXPECT_TRUE(moose_server->handleInitializeRequest(initialize_request, initialize_response));

  // check server knows client has snippet support after initialize request
  EXPECT_TRUE(moose_server->clientSupportsSnippets());

  EXPECT_TRUE(moose_server->getErrors().empty());

  // check the dissected values of the moose_server initialize response

  std::stringstream response_errors;
  int response_id;
  wasp::DataObject server_capabilities;

  EXPECT_TRUE(wasp::lsp::dissectInitializeResponse(
      initialize_response, response_errors, response_id, server_capabilities));

  EXPECT_TRUE(response_errors.str().empty());

  EXPECT_EQ(request_id, response_id);

  EXPECT_EQ(7u, server_capabilities.size());

  EXPECT_TRUE(server_capabilities[wasp::lsp::m_text_doc_sync].is_object());
  const auto & text_doc_sync_caps = *(server_capabilities[wasp::lsp::m_text_doc_sync].to_object());
  EXPECT_EQ(2u, text_doc_sync_caps.size());

  EXPECT_TRUE(text_doc_sync_caps[wasp::lsp::m_open_close].is_bool());
  EXPECT_TRUE(text_doc_sync_caps[wasp::lsp::m_open_close].to_bool());

  EXPECT_TRUE(text_doc_sync_caps[wasp::lsp::m_change].is_int());
  EXPECT_EQ(wasp::lsp::m_change_full, text_doc_sync_caps[wasp::lsp::m_change].to_int());

  EXPECT_TRUE(server_capabilities[wasp::lsp::m_completion_provider].is_object());
  const auto & comp_caps = *(server_capabilities[wasp::lsp::m_completion_provider].to_object());
  EXPECT_EQ(1u, comp_caps.size());

  EXPECT_TRUE(comp_caps[wasp::lsp::m_resolve_provider].is_bool());
  EXPECT_FALSE(comp_caps[wasp::lsp::m_resolve_provider].to_bool());

  EXPECT_TRUE(server_capabilities[wasp::lsp::m_doc_symbol_provider].is_bool());
  EXPECT_TRUE(server_capabilities[wasp::lsp::m_doc_symbol_provider].to_bool());

  EXPECT_TRUE(server_capabilities[wasp::lsp::m_doc_format_provider].is_bool());
  EXPECT_TRUE(server_capabilities[wasp::lsp::m_doc_format_provider].to_bool());

  EXPECT_TRUE(server_capabilities[wasp::lsp::m_definition_provider].is_bool());
  EXPECT_TRUE(server_capabilities[wasp::lsp::m_definition_provider].to_bool());

  EXPECT_TRUE(server_capabilities[wasp::lsp::m_references_provider].is_bool());
  EXPECT_TRUE(server_capabilities[wasp::lsp::m_references_provider].to_bool());

  EXPECT_TRUE(server_capabilities[wasp::lsp::m_hover_provider].is_bool());
  EXPECT_TRUE(server_capabilities[wasp::lsp::m_hover_provider].to_bool());

  // build initialized notification which takes no extra parameters

  wasp::DataObject initialized_notification;
  std::stringstream initialized_errors;

  EXPECT_TRUE(
      wasp::lsp::buildInitializedNotification(initialized_notification, initialized_errors));

  EXPECT_TRUE(initialized_errors.str().empty());

  // handle the built initialized notification with the moose_server

  EXPECT_TRUE(moose_server->handleInitializedNotification(initialized_notification));

  EXPECT_TRUE(moose_server->getErrors().empty());
}

TEST_F(MooseServerTest, DocumentOpenAndDiagnostics)
{
  // didopen test parameters - note input has error with variable u twice

  std::string document_uri = wasp::lsp::m_uri_prefix + std::string("/test/input/path");
  std::string document_language_id = "test_language_id_string";
  int document_version = 1;
  std::string document_text_open = R"INPUT(
[Mesh]
  type = GeneratedMesh
  dim = 1
[]
[Variables]
  [u]
    order = THIRD
    family = MONOMIAL
  []
  [u]
    order = FOURTH
    family = MONOMIAL
  []
[]
[BCs]
  [all]
    type = VacuumBC
    boundary = 'left right'
    variable = u
  []
[]
[Executioner]
  type = Transient
[]
[Problem]
  solve = false
[]
)INPUT";

  // build didopen notification with the test parameters

  wasp::DataObject didopen_notification;
  std::stringstream didopen_errors;

  ASSERT_TRUE(wasp::lsp::buildDidOpenNotification(didopen_notification,
                                                  didopen_errors,
                                                  document_uri,
                                                  document_language_id,
                                                  document_version,
                                                  document_text_open));

  ASSERT_TRUE(didopen_errors.str().empty());

  // handle the built didopen notification with the moose_server

  wasp::DataObject diagnostics_notification;

  ASSERT_TRUE(
      moose_server->handleDidOpenNotification(didopen_notification, diagnostics_notification))
      << moose_server->getErrors();

  ASSERT_TRUE(moose_server->getErrors().empty());

  // check set of messages built from the moose_server diagnostics notification

  std::stringstream diagnostics_errors;
  std::string response_uri;
  wasp::DataArray diagnostics_array;

  EXPECT_TRUE(wasp::lsp::dissectPublishDiagnosticsNotification(
      diagnostics_notification, diagnostics_errors, response_uri, diagnostics_array));

  EXPECT_TRUE(diagnostics_errors.str().empty());

  EXPECT_EQ(document_uri, response_uri);

  EXPECT_EQ(4u, diagnostics_array.size());

  std::ostringstream diagnostics_actual;

  format_diagnostics(diagnostics_array, diagnostics_actual);

  // expected diagnostics with zero-based lines and columns - variable u twice

  std::string diagnostics_expect = R"INPUT(
line:7 column:4 - parameter 'Variables/u/order' supplied multiple times
line:11 column:4 - parameter 'Variables/u/order' supplied multiple times
line:8 column:4 - parameter 'Variables/u/family' supplied multiple times
line:12 column:4 - parameter 'Variables/u/family' supplied multiple times
)INPUT";

  EXPECT_EQ(diagnostics_expect, "\n" + diagnostics_actual.str());
}

TEST_F(MooseServerTest, DocumentOpenAndSymbols)
{
  // symbols test parameters

  int request_id = 2;
  std::string document_uri = wasp::lsp::m_uri_prefix + std::string("/test/input/path");

  // build symbols request with the test parameters

  wasp::DataObject symbols_request;
  std::stringstream symbols_errors;

  EXPECT_TRUE(
      wasp::lsp::buildSymbolsRequest(symbols_request, symbols_errors, request_id, document_uri));

  EXPECT_TRUE(symbols_errors.str().empty());

  // handle the built symbols request with the moose_server

  wasp::DataObject symbols_response;

  EXPECT_TRUE(moose_server->handleSymbolsRequest(symbols_request, symbols_response));

  EXPECT_TRUE(moose_server->getErrors().empty());

  // check set of paths built from the moose_server symbols response

  std::ostringstream paths_actual;

  format_symbolpaths(symbols_response, paths_actual);

  // expected paths with zero-based lines and columns - note variable u twice

  std::string paths_expect = R"INPUT(
/Mesh (1:0)                        detail: GeneratedMesh kind: 23
/Mesh/[ (1:0)                      detail:               kind:  7
/Mesh/decl (1:1)                   detail:               kind:  7
/Mesh/] (1:5)                      detail:               kind:  7
/Mesh/type (2:2)                   detail:               kind: 26
/Mesh/type/decl (2:2)              detail:               kind:  7
/Mesh/type/= (2:7)                 detail:               kind:  7
/Mesh/type/value (2:9)             detail:               kind: 15
/Mesh/dim (3:2)                    detail:               kind: 16
/Mesh/dim/decl (3:2)               detail:               kind:  7
/Mesh/dim/= (3:6)                  detail:               kind:  7
/Mesh/dim/value (3:8)              detail:               kind: 15
/Mesh/term (4:0)                   detail:               kind:  7
/Variables (5:0)                   detail:               kind: 23
/Variables/[ (5:0)                 detail:               kind:  7
/Variables/decl (5:1)              detail:               kind:  7
/Variables/] (5:10)                detail:               kind:  7
/Variables/u (6:2)                 detail:               kind: 23
/Variables/u/[ (6:2)               detail:               kind:  7
/Variables/u/decl (6:3)            detail:               kind:  7
/Variables/u/] (6:4)               detail:               kind:  7
/Variables/u/order (7:4)           detail:               kind: 20
/Variables/u/order/decl (7:4)      detail:               kind:  7
/Variables/u/order/= (7:10)        detail:               kind:  7
/Variables/u/order/value (7:12)    detail:               kind: 15
/Variables/u/family (8:4)          detail:               kind: 20
/Variables/u/family/decl (8:4)     detail:               kind:  7
/Variables/u/family/= (8:11)       detail:               kind:  7
/Variables/u/family/value (8:13)   detail:               kind: 15
/Variables/u/term (9:2)            detail:               kind:  7
/Variables/u (10:2)                detail:               kind: 23
/Variables/u/[ (10:2)              detail:               kind:  7
/Variables/u/decl (10:3)           detail:               kind:  7
/Variables/u/] (10:4)              detail:               kind:  7
/Variables/u/order (11:4)          detail:               kind: 20
/Variables/u/order/decl (11:4)     detail:               kind:  7
/Variables/u/order/= (11:10)       detail:               kind:  7
/Variables/u/order/value (11:12)   detail:               kind: 15
/Variables/u/family (12:4)         detail:               kind: 20
/Variables/u/family/decl (12:4)    detail:               kind:  7
/Variables/u/family/= (12:11)      detail:               kind:  7
/Variables/u/family/value (12:13)  detail:               kind: 15
/Variables/u/term (13:2)           detail:               kind:  7
/Variables/term (14:0)             detail:               kind:  7
/BCs (15:0)                        detail:               kind: 23
/BCs/[ (15:0)                      detail:               kind:  7
/BCs/decl (15:1)                   detail:               kind:  7
/BCs/] (15:4)                      detail:               kind:  7
/BCs/all (16:2)                    detail: VacuumBC      kind: 23
/BCs/all/[ (16:2)                  detail:               kind:  7
/BCs/all/decl (16:3)               detail:               kind:  7
/BCs/all/] (16:6)                  detail:               kind:  7
/BCs/all/type (17:4)               detail:               kind: 26
/BCs/all/type/decl (17:4)          detail:               kind:  7
/BCs/all/type/= (17:9)             detail:               kind:  7
/BCs/all/type/value (17:11)        detail:               kind: 15
/BCs/all/boundary (18:4)           detail:               kind: 18
/BCs/all/boundary/decl (18:4)      detail:               kind:  7
/BCs/all/boundary/= (18:13)        detail:               kind:  7
/BCs/all/boundary/' (18:15)        detail:               kind:  7
/BCs/all/boundary/value (18:16)    detail:               kind: 15
/BCs/all/boundary/value (18:21)    detail:               kind: 15
/BCs/all/boundary/' (18:26)        detail:               kind:  7
/BCs/all/variable (19:4)           detail:               kind: 20
/BCs/all/variable/decl (19:4)      detail:               kind:  7
/BCs/all/variable/= (19:13)        detail:               kind:  7
/BCs/all/variable/value (19:15)    detail:               kind: 15
/BCs/all/term (20:2)               detail:               kind:  7
/BCs/term (21:0)                   detail:               kind:  7
/Executioner (22:0)                detail: Transient     kind: 23
/Executioner/[ (22:0)              detail:               kind:  7
/Executioner/decl (22:1)           detail:               kind:  7
/Executioner/] (22:12)             detail:               kind:  7
/Executioner/type (23:2)           detail:               kind: 26
/Executioner/type/decl (23:2)      detail:               kind:  7
/Executioner/type/= (23:7)         detail:               kind:  7
/Executioner/type/value (23:9)     detail:               kind: 15
/Executioner/term (24:0)           detail:               kind:  7
/Problem (25:0)                    detail:               kind: 23
/Problem/[ (25:0)                  detail:               kind:  7
/Problem/decl (25:1)               detail:               kind:  7
/Problem/] (25:8)                  detail:               kind:  7
/Problem/solve (26:2)              detail:               kind: 17
/Problem/solve/decl (26:2)         detail:               kind:  7
/Problem/solve/= (26:8)            detail:               kind:  7
/Problem/solve/value (26:10)       detail:               kind: 15
/Problem/term (27:0)               detail:               kind:  7
)INPUT";

  EXPECT_EQ(paths_expect, "\n" + paths_actual.str());
}

TEST_F(MooseServerTest, DocumentChangeAndDiagnostics)
{
  // didchange test parameters - note input has error with bad bcs boundary

  std::string document_uri = wasp::lsp::m_uri_prefix + std::string("/test/input/path");
  int document_version = 2;
  int start_line = -1;
  int start_character = -1;
  int end_line = -1;
  int end_character = -1;
  int range_length = -1;
  std::string document_text_change = R"INPUT(
[Mesh]
  type = GeneratedMesh
  dim = 1
[]
[Variables]
  [u]
    order = THIRD
    family = MONOMIAL
  []
  [v]
    order = FOURTH
    family = MONOMIAL
  []
[]
[BCs]
  [all]
    type = VacuumBC
    boundary = 'left right top bottom'
    variable = u
  []
[]
[Executioner]
  type = Transient
[]
[Problem]
  solve = false
[]
)INPUT";

  // build didchange notification with the test parameters

  wasp::DataObject didchange_notification;
  std::stringstream didchange_errors;

  EXPECT_TRUE(wasp::lsp::buildDidChangeNotification(didchange_notification,
                                                    didchange_errors,
                                                    document_uri,
                                                    document_version,
                                                    start_line,
                                                    start_character,
                                                    end_line,
                                                    end_character,
                                                    range_length,
                                                    document_text_change));

  EXPECT_TRUE(didchange_errors.str().empty());

  // handle the built didchange notification with the moose_server

  wasp::DataObject diagnostics_notification;

  EXPECT_TRUE(
      moose_server->handleDidChangeNotification(didchange_notification, diagnostics_notification));

  EXPECT_TRUE(moose_server->getErrors().empty());

  // check set of messages built from the moose_server diagnostics notification

  std::stringstream diagnostics_errors;
  std::string response_uri;
  wasp::DataArray diagnostics_array;

  EXPECT_TRUE(wasp::lsp::dissectPublishDiagnosticsNotification(
      diagnostics_notification, diagnostics_errors, response_uri, diagnostics_array));

  EXPECT_TRUE(diagnostics_errors.str().empty());

  EXPECT_EQ(document_uri, response_uri);

  EXPECT_EQ(7u, diagnostics_array.size());

  std::ostringstream diagnostics_actual;

  format_diagnostics(diagnostics_array, diagnostics_actual);

  // expected diagnostics with zero-based lines and columns - bad bcs boundary

  std::string diagnostics_expect = R"INPUT(
line:18 column:0 - (BCs/all/boundary):
line:18 column:0 -     the following side sets (ids) do not exist on the mesh: top (2), bottom (3)
line:18 column:0 -     MOOSE distinguishes between "node sets" and "side sets" depending on whether
line:18 column:0 -     you are using "Nodal" or "Integrated" BCs respectively. Node sets corresponding
line:18 column:0 -     to your side sets are constructed for you by default.
line:18 column:0 -     Try setting "Mesh/construct_side_list_from_node_list=true" if you see this error.
line:18 column:0 -     Note: If you are running with adaptivity you should prefer using side sets.
)INPUT";

  EXPECT_EQ(diagnostics_expect, "\n" + diagnostics_actual.str());
}

TEST_F(MooseServerTest, DocumentChangeAndSymbols)
{
  // symbols test parameters

  int request_id = 3;
  std::string document_uri = wasp::lsp::m_uri_prefix + std::string("/test/input/path");

  // build symbols request with the test parameters

  wasp::DataObject symbols_request;
  std::stringstream symbols_errors;

  EXPECT_TRUE(
      wasp::lsp::buildSymbolsRequest(symbols_request, symbols_errors, request_id, document_uri));

  EXPECT_TRUE(symbols_errors.str().empty());

  // handle the built symbols request with the moose_server

  wasp::DataObject symbols_response;

  EXPECT_TRUE(moose_server->handleSymbolsRequest(symbols_request, symbols_response));

  EXPECT_TRUE(moose_server->getErrors().empty());

  // check set of paths built from the moose_server symbols response

  std::ostringstream paths_actual;

  format_symbolpaths(symbols_response, paths_actual);

  // expected paths with zero-based lines and columns - note bad bcs boundary

  std::string paths_expect = R"INPUT(
/Mesh (1:0)                        detail: GeneratedMesh kind: 23
/Mesh/[ (1:0)                      detail:               kind:  7
/Mesh/decl (1:1)                   detail:               kind:  7
/Mesh/] (1:5)                      detail:               kind:  7
/Mesh/type (2:2)                   detail:               kind: 26
/Mesh/type/decl (2:2)              detail:               kind:  7
/Mesh/type/= (2:7)                 detail:               kind:  7
/Mesh/type/value (2:9)             detail:               kind: 15
/Mesh/dim (3:2)                    detail:               kind: 16
/Mesh/dim/decl (3:2)               detail:               kind:  7
/Mesh/dim/= (3:6)                  detail:               kind:  7
/Mesh/dim/value (3:8)              detail:               kind: 15
/Mesh/term (4:0)                   detail:               kind:  7
/Variables (5:0)                   detail:               kind: 23
/Variables/[ (5:0)                 detail:               kind:  7
/Variables/decl (5:1)              detail:               kind:  7
/Variables/] (5:10)                detail:               kind:  7
/Variables/u (6:2)                 detail:               kind: 23
/Variables/u/[ (6:2)               detail:               kind:  7
/Variables/u/decl (6:3)            detail:               kind:  7
/Variables/u/] (6:4)               detail:               kind:  7
/Variables/u/order (7:4)           detail:               kind: 20
/Variables/u/order/decl (7:4)      detail:               kind:  7
/Variables/u/order/= (7:10)        detail:               kind:  7
/Variables/u/order/value (7:12)    detail:               kind: 15
/Variables/u/family (8:4)          detail:               kind: 20
/Variables/u/family/decl (8:4)     detail:               kind:  7
/Variables/u/family/= (8:11)       detail:               kind:  7
/Variables/u/family/value (8:13)   detail:               kind: 15
/Variables/u/term (9:2)            detail:               kind:  7
/Variables/v (10:2)                detail:               kind: 23
/Variables/v/[ (10:2)              detail:               kind:  7
/Variables/v/decl (10:3)           detail:               kind:  7
/Variables/v/] (10:4)              detail:               kind:  7
/Variables/v/order (11:4)          detail:               kind: 20
/Variables/v/order/decl (11:4)     detail:               kind:  7
/Variables/v/order/= (11:10)       detail:               kind:  7
/Variables/v/order/value (11:12)   detail:               kind: 15
/Variables/v/family (12:4)         detail:               kind: 20
/Variables/v/family/decl (12:4)    detail:               kind:  7
/Variables/v/family/= (12:11)      detail:               kind:  7
/Variables/v/family/value (12:13)  detail:               kind: 15
/Variables/v/term (13:2)           detail:               kind:  7
/Variables/term (14:0)             detail:               kind:  7
/BCs (15:0)                        detail:               kind: 23
/BCs/[ (15:0)                      detail:               kind:  7
/BCs/decl (15:1)                   detail:               kind:  7
/BCs/] (15:4)                      detail:               kind:  7
/BCs/all (16:2)                    detail: VacuumBC      kind: 23
/BCs/all/[ (16:2)                  detail:               kind:  7
/BCs/all/decl (16:3)               detail:               kind:  7
/BCs/all/] (16:6)                  detail:               kind:  7
/BCs/all/type (17:4)               detail:               kind: 26
/BCs/all/type/decl (17:4)          detail:               kind:  7
/BCs/all/type/= (17:9)             detail:               kind:  7
/BCs/all/type/value (17:11)        detail:               kind: 15
/BCs/all/boundary (18:4)           detail:               kind: 18
/BCs/all/boundary/decl (18:4)      detail:               kind:  7
/BCs/all/boundary/= (18:13)        detail:               kind:  7
/BCs/all/boundary/' (18:15)        detail:               kind:  7
/BCs/all/boundary/value (18:16)    detail:               kind: 15
/BCs/all/boundary/value (18:21)    detail:               kind: 15
/BCs/all/boundary/value (18:27)    detail:               kind: 15
/BCs/all/boundary/value (18:31)    detail:               kind: 15
/BCs/all/boundary/' (18:37)        detail:               kind:  7
/BCs/all/variable (19:4)           detail:               kind: 20
/BCs/all/variable/decl (19:4)      detail:               kind:  7
/BCs/all/variable/= (19:13)        detail:               kind:  7
/BCs/all/variable/value (19:15)    detail:               kind: 15
/BCs/all/term (20:2)               detail:               kind:  7
/BCs/term (21:0)                   detail:               kind:  7
/Executioner (22:0)                detail: Transient     kind: 23
/Executioner/[ (22:0)              detail:               kind:  7
/Executioner/decl (22:1)           detail:               kind:  7
/Executioner/] (22:12)             detail:               kind:  7
/Executioner/type (23:2)           detail:               kind: 26
/Executioner/type/decl (23:2)      detail:               kind:  7
/Executioner/type/= (23:7)         detail:               kind:  7
/Executioner/type/value (23:9)     detail:               kind: 15
/Executioner/term (24:0)           detail:               kind:  7
/Problem (25:0)                    detail:               kind: 23
/Problem/[ (25:0)                  detail:               kind:  7
/Problem/decl (25:1)               detail:               kind:  7
/Problem/] (25:8)                  detail:               kind:  7
/Problem/solve (26:2)              detail:               kind: 17
/Problem/solve/decl (26:2)         detail:               kind:  7
/Problem/solve/= (26:8)            detail:               kind:  7
/Problem/solve/value (26:10)       detail:               kind: 15
/Problem/term (27:0)               detail:               kind:  7
)INPUT";

  EXPECT_EQ(paths_expect, "\n" + paths_actual.str());
}

TEST_F(MooseServerTest, CompletionMeshDefaultedType)
{
  // didchange test parameters - update input to set up autocomplete scenarios

  std::string doc_uri = wasp::lsp::m_uri_prefix + std::string("/test/input/path");
  int doc_version = 3;
  std::string doc_text_change = R"INPUT(
[Mesh]
  dim = 2
  patch_update_strategy = AUTO
  displacements = 'disp_x disp_y'
  parallel_type = REPLICATED

[]
[Variables]
  active = '__all__'
  [u]
  []
  [v]
  []
[]
[AuxVariables]
   [disp_x][]
      [disp_x][]
  [disp_y][]
    [disp_x][]
[]
[BCs]
  [all]
    type = VacuumBC
    boundary = 'left right'
    variable = u
    displacements = 'disp_x'
  []
[]
[Executioner]
  type = Transient
[]
[Problem]
  solve = false
[]
[UserObjects]
  [term_uo]
    type = Terminator
    expression = 'expr'
    error_level = INFO
  []
[]

[Outputs]
  [console]
    type = Console
    system_info = AUX
    execute_on = LINEAR
  []
[]
)INPUT";

  // build didchange notification and handle it with the moose_server

  wasp::DataObject didchange_notification;
  std::stringstream errors;
  wasp::DataObject diagnostics_notification;

  EXPECT_TRUE(wasp::lsp::buildDidChangeNotification(
      didchange_notification, errors, doc_uri, doc_version, -1, -1, -1, -1, -1, doc_text_change));

  EXPECT_TRUE(
      moose_server->handleDidChangeNotification(didchange_notification, diagnostics_notification));

  // completion test parameters - in Mesh default FileMesh already with params
  int request_id = 4;
  int request_line = 6;
  int request_char = 0;
  std::size_t expect_count = 48;
  std::string expect_items = R"INPUT(
label: active                                 text: active = '${1:__all__}'                             desc: If specified only... pos: [6.0]-[6.0] kind:  7 format: snippet
label: add_subdomain_ids                      text: add_subdomain_ids =                                 desc: The listed subdom... pos: [6.0]-[6.0] kind: 14 format: regular
label: add_subdomain_names                    text: add_subdomain_names =                               desc: The listed subdom... pos: [6.0]-[6.0] kind: 14 format: regular
label: allow_renumbering                      text: allow_renumbering = ${1:true}                       desc: If allow_renumber... pos: [6.0]-[6.0] kind:  8 format: snippet
label: alpha_rotation                         text: alpha_rotation =                                    desc: The number of deg... pos: [6.0]-[6.0] kind: 14 format: regular
label: beta_rotation                          text: beta_rotation =                                     desc: The number of deg... pos: [6.0]-[6.0] kind: 14 format: regular
label: block_id                               text: block_id =                                          desc: IDs of the block ... pos: [6.0]-[6.0] kind: 14 format: regular
label: block_name                             text: block_name =                                        desc: Names of the bloc... pos: [6.0]-[6.0] kind: 14 format: regular
label: boundary_id                            text: boundary_id =                                       desc: IDs of the bounda... pos: [6.0]-[6.0] kind: 14 format: regular
label: boundary_name                          text: boundary_name =                                     desc: Names of the boun... pos: [6.0]-[6.0] kind: 14 format: regular
label: build_all_side_lowerd_mesh             text: build_all_side_lowerd_mesh = ${1:false}             desc: True to build the... pos: [6.0]-[6.0] kind:  8 format: snippet
label: centroid_partitioner_direction         text: centroid_partitioner_direction =                    desc: Specifies the sor... pos: [6.0]-[6.0] kind: 13 format: regular
label: clear_spline_nodes                     text: clear_spline_nodes = ${1:false}                     desc: If clear_spline_n... pos: [6.0]-[6.0] kind:  8 format: snippet
label: construct_node_list_from_side_list     text: construct_node_list_from_side_list = ${1:true}      desc: Whether or not to... pos: [6.0]-[6.0] kind:  8 format: snippet
label: construct_side_list_from_node_list     text: construct_side_list_from_node_list = ${1:false}     desc: If true, construc... pos: [6.0]-[6.0] kind:  8 format: snippet
label: control_tags                           text: control_tags =                                      desc: Adds user-defined... pos: [6.0]-[6.0] kind: 14 format: regular
label: coord_block                            text: coord_block =                                       desc: Block IDs for the... pos: [6.0]-[6.0] kind: 14 format: regular
label: coord_type                             text: coord_type = '${1:XYZ}'                             desc: Type of the coord... pos: [6.0]-[6.0] kind: 13 format: snippet
label: enable                                 text: enable = ${1:true}                                  desc: Set the enabled s... pos: [6.0]-[6.0] kind:  8 format: snippet
label: file                                   text: file =                                              desc: The name of the m... pos: [6.0]-[6.0] kind: 23 format: regular
label: gamma_rotation                         text: gamma_rotation =                                    desc: The number of deg... pos: [6.0]-[6.0] kind: 14 format: regular
label: ghosted_boundaries                     text: ghosted_boundaries =                                desc: Boundaries to be ... pos: [6.0]-[6.0] kind: 14 format: regular
label: ghosted_boundaries_inflation           text: ghosted_boundaries_inflation =                      desc: If you are using ... pos: [6.0]-[6.0] kind: 14 format: regular
label: ghosting_patch_size                    text: ghosting_patch_size =                               desc: The number of nea... pos: [6.0]-[6.0] kind: 14 format: regular
label: inactive                               text: inactive =                                          desc: If specified bloc... pos: [6.0]-[6.0] kind:  7 format: regular
label: include_local_in_ghosting              text: include_local_in_ghosting = ${1:false}              desc: Boolean used to t... pos: [6.0]-[6.0] kind:  8 format: snippet
label: length_unit                            text: length_unit =                                       desc: How much distance... pos: [6.0]-[6.0] kind: 14 format: regular
label: max_leaf_size                          text: max_leaf_size = ${1:10}                             desc: The maximum numbe... pos: [6.0]-[6.0] kind: 14 format: snippet
label: nemesis                                text: nemesis = ${1:false}                                desc: If nemesis=true a... pos: [6.0]-[6.0] kind:  8 format: snippet
label: output_ghosting                        text: output_ghosting = ${1:false}                        desc: Boolean to turn o... pos: [6.0]-[6.0] kind:  8 format: snippet
label: partitioner                            text: partitioner = ${1:default}                          desc: Specifies a mesh ... pos: [6.0]-[6.0] kind: 13 format: snippet
label: patch_size                             text: patch_size = ${1:40}                                desc: The number of nod... pos: [6.0]-[6.0] kind: 14 format: snippet
label: rz_coord_axis                          text: rz_coord_axis = ${1:Y}                              desc: The rotation axis... pos: [6.0]-[6.0] kind: 13 format: snippet
label: rz_coord_blocks                        text: rz_coord_blocks =                                   desc: Blocks using gene... pos: [6.0]-[6.0] kind: 14 format: regular
label: rz_coord_directions                    text: rz_coord_directions =                               desc: Axis directions f... pos: [6.0]-[6.0] kind: 14 format: regular
label: rz_coord_origins                       text: rz_coord_origins =                                  desc: Axis origin point... pos: [6.0]-[6.0] kind: 14 format: regular
label: second_order                           text: second_order = ${1:false}                           desc: Converts a first ... pos: [6.0]-[6.0] kind:  8 format: snippet
label: skip_deletion_repartition_after_refine text: skip_deletion_repartition_after_refine = ${1:false} desc: If the flag is tr... pos: [6.0]-[6.0] kind:  8 format: snippet
label: skip_partitioning                      text: skip_partitioning = ${1:false}                      desc: If true the mesh ... pos: [6.0]-[6.0] kind:  8 format: snippet
label: skip_refine_when_use_split             text: skip_refine_when_use_split = ${1:true}              desc: True to skip unif... pos: [6.0]-[6.0] kind:  8 format: snippet
label: split_file                             text: split_file =                                        desc: Optional name of ... pos: [6.0]-[6.0] kind: 14 format: regular
label: type                                   text: type = ${1:FileMesh}                                desc: A string represen... pos: [6.0]-[6.0] kind: 25 format: snippet
label: uniform_refine                         text: uniform_refine = ${1:0}                             desc: Specify the level... pos: [6.0]-[6.0] kind: 14 format: snippet
label: up_direction                           text: up_direction =                                      desc: Specify what axis... pos: [6.0]-[6.0] kind: 13 format: regular
label: use_displaced_mesh                     text: use_displaced_mesh = ${1:true}                      desc: Create the displa... pos: [6.0]-[6.0] kind:  8 format: snippet
label: use_split                              text: use_split = ${1:false}                              desc: Use split distrib... pos: [6.0]-[6.0] kind:  8 format: snippet
label: *                                      text: [block_name]\n  type = \n  $0\n[]                   desc: custom user named... pos: [6.0]-[6.0] kind:  6 format: snippet
label: Partitioner                            text: [Partitioner]\n  type = \n  $0\n[]                  desc: application named... pos: [6.0]-[6.0] kind: 22 format: snippet
)INPUT";

  check_completions(request_id, doc_uri, request_line, request_char, expect_count, expect_items);
}

TEST_F(MooseServerTest, CompletionDocumentRootLevel)
{
  // completion test parameters - at document root level outside of all blocks
  std::string doc_uri = wasp::lsp::m_uri_prefix + std::string("/test/input/path");
  int request_id = 5;
  int request_line = 42;
  int request_char = 0;
  std::size_t expect_count = 50;
  std::string expect_items = R"INPUT(
label: active                           text: active = '${1:__all__}'                      desc: If specified only... pos: [42.0]-[42.0] kind:  7 format: snippet
label: inactive                         text: inactive =                                   desc: If specified bloc... pos: [42.0]-[42.0] kind:  7 format: regular
label: Adaptivity                       text: [Adaptivity]\n  $0\n[]                       desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: Application                      text: [Application]\n  $0\n[]                      desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: AuxKernels                       text: [AuxKernels]\n  $0\n[]                       desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: AuxScalarKernels                 text: [AuxScalarKernels]\n  $0\n[]                 desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: AuxVariables                     text: [AuxVariables]\n  $0\n[]                     desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: BCs                              text: [BCs]\n  $0\n[]                              desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: Bounds                           text: [Bounds]\n  $0\n[]                           desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: Constraints                      text: [Constraints]\n  $0\n[]                      desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: Controls                         text: [Controls]\n  $0\n[]                         desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: DGKernels                        text: [DGKernels]\n  $0\n[]                        desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: Dampers                          text: [Dampers]\n  $0\n[]                          desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: Debug                            text: [Debug]\n  $0\n[]                            desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: DeprecatedBlock                  text: [DeprecatedBlock]\n  $0\n[]                  desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: DiracKernels                     text: [DiracKernels]\n  $0\n[]                     desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: Distributions                    text: [Distributions]\n  $0\n[]                    desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: Executioner                      text: [Executioner]\n  type = \n  $0\n[]           desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: Executors                        text: [Executors]\n  $0\n[]                        desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: FVBCs                            text: [FVBCs]\n  $0\n[]                            desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: FVICs                            text: [FVICs]\n  $0\n[]                            desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: FVInterfaceKernels               text: [FVInterfaceKernels]\n  $0\n[]               desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: FVKernels                        text: [FVKernels]\n  $0\n[]                        desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: Functions                        text: [Functions]\n  $0\n[]                        desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: FunctorMaterials                 text: [FunctorMaterials]\n  $0\n[]                 desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: GlobalParams                     text: [GlobalParams]\n  $0\n[]                     desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: ICs                              text: [ICs]\n  $0\n[]                              desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: InterfaceKernels                 text: [InterfaceKernels]\n  $0\n[]                 desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: Kernels                          text: [Kernels]\n  $0\n[]                          desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: Materials                        text: [Materials]\n  $0\n[]                        desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: Mesh                             text: [Mesh]\n  file = \n  $0\n[]                  desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: MeshDivisions                    text: [MeshDivisions]\n  $0\n[]                    desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: MultiApps                        text: [MultiApps]\n  $0\n[]                        desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: NodalKernels                     text: [NodalKernels]\n  $0\n[]                     desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: NodalNormals                     text: [NodalNormals]\n  $0\n[]                     desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: Outputs                          text: [Outputs]\n  $0\n[]                          desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: Physics                          text: [Physics]\n  $0\n[]                          desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: Positions                        text: [Positions]\n  $0\n[]                        desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: Postprocessors                   text: [Postprocessors]\n  $0\n[]                   desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: Preconditioning                  text: [Preconditioning]\n  $0\n[]                  desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: Problem                          text: [Problem]\n  $0\n[]                          desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: ProjectedStatefulMaterialStorage text: [ProjectedStatefulMaterialStorage]\n  $0\n[] desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: Reporters                        text: [Reporters]\n  $0\n[]                        desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: Samplers                         text: [Samplers]\n  $0\n[]                         desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: ScalarKernels                    text: [ScalarKernels]\n  $0\n[]                    desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: Times                            text: [Times]\n  $0\n[]                            desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: Transfers                        text: [Transfers]\n  $0\n[]                        desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: UserObjects                      text: [UserObjects]\n  $0\n[]                      desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: Variables                        text: [Variables]\n  $0\n[]                        desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
label: VectorPostprocessors             text: [VectorPostprocessors]\n  $0\n[]             desc: application named... pos: [42.0]-[42.0] kind: 22 format: snippet
)INPUT";
  check_completions(request_id, doc_uri, request_line, request_char, expect_count, expect_items);
}

TEST_F(MooseServerTest, CompletionValueActiveBlocks)
{
  // completion test parameters - on active parameter value in Variables block
  std::string doc_uri = wasp::lsp::m_uri_prefix + std::string("/test/input/path");
  int request_id = 6;
  int request_line = 9;
  int request_char = 12;
  std::size_t expect_count = 2;
  std::string expect_items = R"INPUT(
label: u text: u desc: subblock name pos: [9.12]-[9.19] kind:  7 format: regular
label: v text: v desc: subblock name pos: [9.12]-[9.19] kind:  7 format: regular
)INPUT";
  check_completions(request_id, doc_uri, request_line, request_char, expect_count, expect_items);
}

TEST_F(MooseServerTest, CompletionValueBooleanParam)
{
  // completion test parameters - on boolean value of solve param from Problem
  std::string doc_uri = wasp::lsp::m_uri_prefix + std::string("/test/input/path");
  int request_id = 7;
  int request_line = 33;
  int request_char = 10;
  std::size_t expect_count = 2;
  std::string expect_items = R"INPUT(
label: false text: false desc:  pos: [33.10]-[33.15] kind:  8 format: regular
label: true  text: true  desc:  pos: [33.10]-[33.15] kind:  8 format: regular
)INPUT";
  check_completions(request_id, doc_uri, request_line, request_char, expect_count, expect_items);
}

TEST_F(MooseServerTest, CompletionValueEnumsAndDocs)
{
  // completion test parameters - on error_level enum in Terminator UserObject
  std::string doc_uri = wasp::lsp::m_uri_prefix + std::string("/test/input/path");
  int request_id = 8;
  int request_line = 39;
  int request_char = 18;
  std::size_t expect_count = 4;
  std::string expect_items = R"INPUT(
label: ERROR   text: ERROR   desc: Throw a MOOSE err... pos: [39.18]-[39.22] kind: 20 format: regular
label: INFO    text: INFO    desc: Output an informa... pos: [39.18]-[39.22] kind: 20 format: regular
label: NONE    text: NONE    desc: No message will b... pos: [39.18]-[39.22] kind: 20 format: regular
label: WARNING text: WARNING desc: Output a warning ... pos: [39.18]-[39.22] kind: 20 format: regular
)INPUT";
  check_completions(request_id, doc_uri, request_line, request_char, expect_count, expect_items);
}

TEST_F(MooseServerTest, CompletionValueAllowedTypes)
{
  // completion test parameters - on type parameter value in Executioner block
  std::string doc_uri = wasp::lsp::m_uri_prefix + std::string("/test/input/path");
  int request_id = 9;
  int request_line = 30;
  int request_char = 9;
  std::size_t expect_count = 5;
  std::string expect_items = R"INPUT(
label: Eigenvalue         text: Eigenvalue                       desc: Eigenvalue solves... pos: [30.9]-[30.18] kind: 25 format: regular
label: InversePowerMethod text: InversePowerMethod\nbx_norm = \n desc: Inverse power met... pos: [30.9]-[30.18] kind: 25 format: regular
label: NonlinearEigen     text: NonlinearEigen\nbx_norm = \n     desc: Executioner for e... pos: [30.9]-[30.18] kind: 25 format: regular
label: Steady             text: Steady                           desc: Executioner for s... pos: [30.9]-[30.18] kind: 25 format: regular
label: Transient          text: Transient                        desc: Executioner for t... pos: [30.9]-[30.18] kind: 25 format: regular
)INPUT";
  check_completions(request_id, doc_uri, request_line, request_char, expect_count, expect_items);
}

TEST_F(MooseServerTest, CompletionValueInputLookups)
{
  // completion test parameters - on displacements parameter value in VacuumBC
  std::string doc_uri = wasp::lsp::m_uri_prefix + std::string("/test/input/path");
  int request_id = 10;
  int request_line = 26;
  int request_char = 21;
  std::size_t expect_count = 4;
  std::string expect_items = R"INPUT(
label: disp_x text: disp_x desc: from /AuxVariables/* pos: [26.21]-[26.27] kind: 18 format: regular
label: disp_y text: disp_y desc: from /AuxVariables/* pos: [26.21]-[26.27] kind: 18 format: regular
label: u      text: u      desc: from /Variables/*    pos: [26.21]-[26.27] kind: 18 format: regular
label: v      text: v      desc: from /Variables/*    pos: [26.21]-[26.27] kind: 18 format: regular
)INPUT";
  check_completions(request_id, doc_uri, request_line, request_char, expect_count, expect_items);
}

TEST_F(MooseServerTest, DefinitionObjectTypeSource)
{
  // definition test parameters - on Transient.C defined object type Transient

  int request_id = 11;
  std::string doc_uri = wasp::lsp::m_uri_prefix + std::string("/test/input/path");
  int line = 30;
  int character = 9;

  // build definition request with the test parameters

  wasp::DataObject definition_request;
  std::stringstream definition_errors;

  EXPECT_TRUE(wasp::lsp::buildDefinitionRequest(
      definition_request, definition_errors, request_id, doc_uri, line, character));

  EXPECT_TRUE(definition_errors.str().empty());

  // handle the built definition request with the moose_server

  wasp::DataObject definition_response;

  EXPECT_TRUE(moose_server->handleDefinitionRequest(definition_request, definition_response));

  EXPECT_TRUE(moose_server->getErrors().empty());

  // check the dissected values of the moose_server definition response

  std::stringstream response_errors;
  int response_id;
  wasp::DataArray locations_array;

  EXPECT_TRUE(wasp::lsp::dissectLocationsResponse(
      definition_response, response_errors, response_id, locations_array));

  EXPECT_TRUE(response_errors.str().empty());

  EXPECT_EQ(request_id, response_id);

  EXPECT_EQ(1u, locations_array.size());

  std::ostringstream locations_actual;

  format_locations(locations_array, locations_actual);

  // expected locations with zero-based lines and columns

  std::string locations_expect = R"INPUT(
document_uri: "file://...absolute.../framework/src/executioners/Transient.C"    location_start: [16.0]    location_end: [16.1000]
)INPUT";

  EXPECT_EQ(locations_expect, "\n" + locations_actual.str());
}

TEST_F(MooseServerTest, DefinitionInputFileLookups)
{
  // definition test parameters - on AuxVariables defined displacements disp_x

  int request_id = 12;
  std::string doc_uri = wasp::lsp::m_uri_prefix + std::string("/test/input/path");
  int line = 26;
  int character = 21;

  // build definition request with the test parameters

  wasp::DataObject definition_request;
  std::stringstream definition_errors;

  EXPECT_TRUE(wasp::lsp::buildDefinitionRequest(
      definition_request, definition_errors, request_id, doc_uri, line, character));

  EXPECT_TRUE(definition_errors.str().empty());

  // handle the built definition request with the moose_server

  wasp::DataObject definition_response;

  EXPECT_TRUE(moose_server->handleDefinitionRequest(definition_request, definition_response));

  EXPECT_TRUE(moose_server->getErrors().empty());

  // check the dissected values of the moose_server definition response

  std::stringstream response_errors;
  int response_id;
  wasp::DataArray locations_array;

  EXPECT_TRUE(wasp::lsp::dissectLocationsResponse(
      definition_response, response_errors, response_id, locations_array));

  EXPECT_TRUE(response_errors.str().empty());

  EXPECT_EQ(request_id, response_id);

  EXPECT_EQ(3u, locations_array.size());

  std::ostringstream locations_actual;

  format_locations(locations_array, locations_actual);

  // expected locations with zero-based lines and columns

  std::string locations_expect = R"INPUT(
document_uri: "file:///test/input/path"    location_start: [16.4]    location_end: [16.10]
document_uri: "file:///test/input/path"    location_start: [17.7]    location_end: [17.13]
document_uri: "file:///test/input/path"    location_start: [19.5]    location_end: [19.11]
)INPUT";

  EXPECT_EQ(locations_expect, "\n" + locations_actual.str());
}

TEST_F(MooseServerTest, HoverDocumentationRequests)
{
  // check hover 01 - on boundary parameter key in BCs block of VacuumBC type
  std::string doc_uri = wasp::lsp::m_uri_prefix + std::string("/test/input/path");
  int request_id = 13;
  int request_line = 24;
  int request_char = 10;
  std::string expect_text = "The list of boundary IDs from the mesh where this object applies";
  check_hover(request_id, doc_uri, request_line, request_char, expect_text);

  // check hover 02 - on value of VacuumBC for type parameter in BCs subblock
  request_id = 14;
  request_line = 23;
  request_char = 15;
  expect_text = "Vacuum boundary condition for diffusion.";
  check_hover(request_id, doc_uri, request_line, request_char, expect_text);

  // check hover 03 - on error_level MooseEnum INFO with documentation string
  request_id = 15;
  request_line = 39;
  request_char = 20;
  expect_text = "Output an information message once.";
  check_hover(request_id, doc_uri, request_line, request_char, expect_text);

  // check hover 04 - on system_info MultiMooseEnum AUX with no documentation
  request_id = 16;
  request_line = 46;
  request_char = 20;
  expect_text = "";
  check_hover(request_id, doc_uri, request_line, request_char, expect_text);

  // check hover 05 - on execute_on ExecFlagEnum LINEAR with no documentation
  request_id = 17;
  request_line = 47;
  request_char = 20;
  expect_text = "";
  check_hover(request_id, doc_uri, request_line, request_char, expect_text);

  // check hover 06 - on Output subblock name which is unsupported hover type
  request_id = 18;
  request_line = 44;
  request_char = 7;
  expect_text = "";
  check_hover(request_id, doc_uri, request_line, request_char, expect_text);
}

TEST_F(MooseServerTest, CompletionPartialInputCases)
{
  // didchange test parameters - update for partial input completion checking
  std::string doc_uri = wasp::lsp::m_uri_prefix + std::string("/test/input/path");
  int doc_version = 4;
  std::string doc_text_change = R"INPUT(
[Mesh]
  type = GeneratedMesh
  ghos
[]
[Variables]
  [u]
    [
    []
  []
  [v]
  []
[]
[Kernels]
  [diff]
    type = Diffusion
    variable =
  []
[]
[Executioner]
  type = Steady
  [Tim
  []
[]
[Outputs]
  [out]
    type = Exodus
    output_dimension =
    lin
)INPUT";

  // build didchange notification and handle it with the moose_server
  wasp::DataObject didchange_notification;
  std::stringstream errors;
  wasp::DataObject diagnostics_notification;
  EXPECT_TRUE(wasp::lsp::buildDidChangeNotification(
      didchange_notification, errors, doc_uri, doc_version, -1, -1, -1, -1, -1, doc_text_change));
  EXPECT_TRUE(
      moose_server->handleDidChangeNotification(didchange_notification, diagnostics_notification));

  // check partial input completion 01 - on incomplete ghos parameter in Mesh
  int request_id = 19;
  int request_line = 3;
  int request_char = 6;
  std::size_t expect_count = 4;
  std::string expect_items = R"INPUT(
label: ghosted_boundaries           text: ghosted_boundaries =            desc: Boundaries to be ... pos: [3.2]-[3.6] kind: 14 format: regular
label: ghosted_boundaries_inflation text: ghosted_boundaries_inflation =  desc: If you are using ... pos: [3.2]-[3.6] kind: 14 format: regular
label: ghosting_patch_size          text: ghosting_patch_size =           desc: The number of nea... pos: [3.2]-[3.6] kind: 14 format: regular
label: *                            text: [ghos]\n  type = \n  $0\n[]     desc: custom user named... pos: [3.2]-[3.6] kind:  6 format: snippet
)INPUT";
  check_completions(request_id, doc_uri, request_line, request_char, expect_count, expect_items);

  // check partial input completion 02 - on missing block name in Variables/u
  request_id = 20;
  request_line = 7;
  request_char = 5;
  expect_count = 2;
  expect_items = R"INPUT(
label: FVInitialCondition text: FVInitialCondition]\n  type = \n  $0\n[] desc: application named... pos: [7.5]-[7.6] kind: 22 format: snippet
label: InitialCondition   text: InitialCondition]\n  type = \n  $0\n[]   desc: application named... pos: [7.5]-[7.6] kind: 22 format: snippet
)INPUT";
  check_completions(request_id, doc_uri, request_line, request_char, expect_count, expect_items);

  // check partial input completion 03 - on missing lookup value for variable
  request_id = 21;
  request_line = 16;
  request_char = 15;
  expect_count = 2;
  expect_items = R"INPUT(
label: u text: u desc: from /Variables/* pos: [16.15]-[16.15] kind: 18 format: regular
label: v text: v desc: from /Variables/* pos: [16.15]-[16.15] kind: 18 format: regular
)INPUT";
  check_completions(request_id, doc_uri, request_line, request_char, expect_count, expect_items);

  // check partial input completion 04 - on incomplete Executioner block name
  request_id = 22;
  request_line = 21;
  request_char = 6;
  expect_count = 3;
  expect_items = R"INPUT(
label: TimeIntegrator  text: TimeIntegrator]\n  type = \n  $0\n[] desc: application named... pos: [21.3]-[21.6] kind: 22 format: snippet
label: TimeStepper     text: TimeStepper]\n  type = \n  $0\n[]    desc: application named... pos: [21.3]-[21.6] kind: 22 format: snippet
label: TimeSteppers    text: TimeSteppers]\n  $0\n[]              desc: application named... pos: [21.3]-[21.6] kind: 22 format: snippet
)INPUT";
  check_completions(request_id, doc_uri, request_line, request_char, expect_count, expect_items);

  // check partial input completion 05 - on missing value with unclosed block
  request_id = 23;
  request_line = 27;
  request_char = 23;
  expect_count = 5;
  expect_items = R"INPUT(
label: 1                 text: 1                 desc:  pos: [27.23]-[27.23] kind: 20 format: regular
label: 2                 text: 2                 desc:  pos: [27.23]-[27.23] kind: 20 format: regular
label: 3                 text: 3                 desc:  pos: [27.23]-[27.23] kind: 20 format: regular
label: DEFAULT           text: DEFAULT           desc:  pos: [27.23]-[27.23] kind: 20 format: regular
label: PROBLEM_DIMENSION text: PROBLEM_DIMENSION desc:  pos: [27.23]-[27.23] kind: 20 format: regular
)INPUT";
  check_completions(request_id, doc_uri, request_line, request_char, expect_count, expect_items);

  // check partial input completion 06 - on incomplete decl in unclosed block
  request_id = 24;
  request_line = 28;
  request_char = 7;
  expect_count = 3;
  expect_items = R"INPUT(
label: linear_residual_dt_divisor text: linear_residual_dt_divisor = ${1:1000} desc: Number of divisio... pos: [28.4]-[28.7] kind: 14 format: snippet
label: linear_residual_end_time   text: linear_residual_end_time =             desc: Specifies an end ... pos: [28.4]-[28.7] kind: 14 format: regular
label: linear_residual_start_time text: linear_residual_start_time =           desc: Specifies a start... pos: [28.4]-[28.7] kind: 14 format: regular
)INPUT";
  check_completions(request_id, doc_uri, request_line, request_char, expect_count, expect_items);
}

TEST_F(MooseServerTest, DocumentReferencesRequest)
{
  // didchange test parameters - update input to set up document references
  std::string doc_uri = wasp::lsp::m_uri_prefix + std::string("/test/input/path");
  int doc_version = 5;
  std::string doc_text_change = R"INPUT(
[Mesh]
  type = GeneratedMesh
  dim = 1
[]
[Variables]
  [u][]
  [v][]
[]
[Kernels]
  [diff]
    type = Diffusion
    variable = "u"
  []
[]
[BCs]
  [left]
    type = VacuumBC
    boundary = left
    variable = u
    prop_getter_suffix = u
  []
  [right]
    type = VacuumBC
    boundary = right
    variable = v
    displacements = 'u v u v u'
  []
[]
[Executioner]
  type = Transient
[]
[Problem]
  solve = false
[]
)INPUT";

  // build didchange notification from parameters and handle it with server
  wasp::DataObject didchange_notification, diagnostics_notification;
  std::stringstream errors;
  EXPECT_TRUE(wasp::lsp::buildDidChangeNotification(
      didchange_notification, errors, doc_uri, doc_version, -1, -1, -1, -1, -1, doc_text_change));
  EXPECT_TRUE(
      moose_server->handleDidChangeNotification(didchange_notification, diagnostics_notification));

  // references test parameters - on subblock declarator of variable name u
  int request_id = 25;
  int request_line = 6;
  int request_char = 4;
  bool incl_decl = true;

  // build references request with the test parameters for the moose_server
  wasp::DataObject references_request;
  std::stringstream references_errors;
  EXPECT_TRUE(wasp::lsp::buildReferencesRequest(references_request,
                                                references_errors,
                                                request_id,
                                                doc_uri,
                                                request_line,
                                                request_char,
                                                incl_decl));
  EXPECT_TRUE(references_errors.str().empty());

  // handle references request built from parameters using the moose_server
  wasp::DataObject references_response;
  EXPECT_TRUE(moose_server->handleReferencesRequest(references_request, references_response));
  EXPECT_TRUE(moose_server->getErrors().empty());

  // check dissected values of references response sent by the moose_server
  std::stringstream response_errors;
  int response_id;
  wasp::DataArray locations_array;
  EXPECT_TRUE(wasp::lsp::dissectLocationsResponse(
      references_response, response_errors, response_id, locations_array));
  EXPECT_TRUE(response_errors.str().empty());
  EXPECT_EQ(request_id, response_id);
  EXPECT_EQ(6u, locations_array.size());

  // make formatted list of response references and check it is as expected
  std::ostringstream locations_actual;
  format_locations(locations_array, locations_actual);
  std::string locations_expect = R"INPUT(
document_uri: "file:///test/input/path"    location_start: [6.3]    location_end: [6.4]
document_uri: "file:///test/input/path"    location_start: [12.15]    location_end: [12.18]
document_uri: "file:///test/input/path"    location_start: [19.15]    location_end: [19.16]
document_uri: "file:///test/input/path"    location_start: [26.21]    location_end: [26.22]
document_uri: "file:///test/input/path"    location_start: [26.25]    location_end: [26.26]
document_uri: "file:///test/input/path"    location_start: [26.29]    location_end: [26.30]
)INPUT";
  EXPECT_EQ(locations_expect, "\n" + locations_actual.str());
}

TEST_F(MooseServerTest, DocumentFormattingRequest)
{
  // put variables file on disk which will be included from base input file

  std::ofstream include_variables("include_variables.i");
  include_variables << "[Variables]\n  [u]\n  []\n[]\n";
  include_variables.close();

  // didchange test parameters - update input to set up document formatting

  std::string doc_uri = wasp::lsp::m_uri_prefix + std::string("/test/input/path");
  int doc_version = 6;
  std::string doc_text_change = R"INPUT(

num_dim = 2

[Mesh]
type=GeneratedMesh   dim   =  ${num_dim}
    []

[Functions]
[./multi_line_indent_increase_01]
type = PiecewiseConstant
xy_data = "0 0.0
           1 1.0
           10 3.0
           100 2.0
           1000 4.0"
[../]
[./multi_line_indent_increase_02]
type = PiecewiseConstant
xy_data = " 0 0.0
            1 11.0
           10 33.0
          100 22.30
         1000 42.210"
[../]
[./multi_line_indent_decrease_01]
            type = PiecewiseConstant
            xy_data = "100 1.0
                       200 12.0
                       300 313.0
                       400 4514.0
                       500 45615.0"
[../]
[./multi_line_indent_decrease_02]
            type = PiecewiseConstant
            xy_data = "100 1.0
  200 12.0
                       500 45615.0"
[../]

[./double_quoted_string_reflowed]
type = ParsedFunction
expression = "0.1 - 2.0 * 0.2 * x^1 + 3.0 * 0.3 * x^2 - 4.0 * 0.4 * x^3 + 5.0 * 0.5 * x^4 - 6.0 * 0.6 * x^5 + 7.0 * 0.7 * x^6 - 8.0 * 0.8 * x^7 + 9.0 * 0.9 * x^8 - 10.0 * 1.0 * x^9"
[../]
[./single_quoted_string_constant]
type = ParsedFunction
expression = '0.1 - 2.0 * 0.2 * x^1 + 3.0 * 0.3 * x^2 - 4.0 * 0.4 * x^3 + 5.0 * 0.5 * x^4 - 6.0 * 0.6 * x^5 + 7.0 * 0.7 * x^6 - 8.0 * 0.8 * x^7 + 9.0 * 0.9 * x^8 - 10.0 * 1.0 * x^9'
[../]
[]

    !include    include_variables.i    # inline comment 01

        # normal comment 01
   [Problem] solve=  false []

[Executioner]        # inline comment 02
# normal comment 02
        type   =Transient    # inline comment 03
      [../]

)INPUT";

  // build didchange notification and handle it with the moose_server

  wasp::DataObject didchange_notification;
  std::stringstream errors;
  wasp::DataObject diagnostics_notification;

  EXPECT_TRUE(wasp::lsp::buildDidChangeNotification(
      didchange_notification, errors, doc_uri, doc_version, -1, -1, -1, -1, -1, doc_text_change));

  EXPECT_TRUE(
      moose_server->handleDidChangeNotification(didchange_notification, diagnostics_notification));

  // formatting test parameters

  int request_id = 26;
  int tab_size = 4;
  int insert_spaces = true;

  // build formatting request with the test parameters

  wasp::DataObject formatting_request;
  std::stringstream formatting_errors;

  EXPECT_TRUE(wasp::lsp::buildFormattingRequest(
      formatting_request, formatting_errors, request_id, doc_uri, tab_size, insert_spaces));

  EXPECT_TRUE(formatting_errors.str().empty());

  // handle the built formatting request with the moose_server

  wasp::DataObject formatting_response;

  EXPECT_TRUE(moose_server->handleFormattingRequest(formatting_request, formatting_response));

  EXPECT_TRUE(moose_server->getErrors().empty());

  // check the dissected values of the moose_server formatting response

  std::stringstream response_errors;
  int response_id;
  wasp::DataArray textedits_array;

  EXPECT_TRUE(wasp::lsp::dissectFormattingResponse(
      formatting_response, response_errors, response_id, textedits_array));

  EXPECT_TRUE(response_errors.str().empty());

  EXPECT_EQ(request_id, response_id);

  EXPECT_EQ(1u, textedits_array.size());

  std::ostringstream textedits_actual;

  format_textedits(textedits_array, textedits_actual);

  // expected textedits with zero-based lines and columns

  std::string textedits_expect = R"INPUT(
textedit_position: [2.0]-[58.11]
textedit_new_text:
num_dim = 2

[Mesh]
    type = GeneratedMesh
    dim = ${num_dim}
[]

[Functions]
    [multi_line_indent_increase_01]
        type = PiecewiseConstant
        xy_data = "0 0.0
                   1 1.0
                   10 3.0
                   100 2.0
                   1000 4.0"
    []
    [multi_line_indent_increase_02]
        type = PiecewiseConstant
        xy_data = " 0 0.0
                    1 11.0
                   10 33.0
                  100 22.30
                 1000 42.210"
    []
    [multi_line_indent_decrease_01]
        type = PiecewiseConstant
        xy_data = "100 1.0
                   200 12.0
                   300 313.0
                   400 4514.0
                   500 45615.0"
    []
    [multi_line_indent_decrease_02]
        type = PiecewiseConstant
        xy_data = "100 1.0
200 12.0
                   500 45615.0"
    []

    [double_quoted_string_reflowed]
        type = ParsedFunction
        expression = "0.1 - 2.0 * 0.2 * x^1 + 3.0 * 0.3 * x^2 - 4.0 * 0.4 * x^3 + 5.0 * 0.5 * x^4 - "
                     "6.0 * 0.6 * x^5 + 7.0 * 0.7 * x^6 - 8.0 * 0.8 * x^7 + 9.0 * 0.9 * x^8 - 10.0 * "
                     "1.0 * x^9"
    []
    [single_quoted_string_constant]
        type = ParsedFunction
        expression = '0.1 - 2.0 * 0.2 * x^1 + 3.0 * 0.3 * x^2 - 4.0 * 0.4 * x^3 + 5.0 * 0.5 * x^4 - 6.0 * 0.6 * x^5 + 7.0 * 0.7 * x^6 - 8.0 * 0.8 * x^7 + 9.0 * 0.9 * x^8 - 10.0 * 1.0 * x^9'
    []
[]

!include include_variables.i # inline comment 01

# normal comment 01
[Problem]
    solve = false
[]

[Executioner] # inline comment 02
    # normal comment 02
    type = Transient # inline comment 03
[]
)INPUT";

  EXPECT_EQ(textedits_expect, "\n" + textedits_actual.str());

  // remove variables file from disk that was included from base input file

  std::remove("include_variables.i");
}

TEST_F(MooseServerTest, DiagnosticsEmptyMessageSkip)
{
  // didchange test parameters - create empty diagnostic which is not added
  std::string doc_uri = wasp::lsp::m_uri_prefix + std::string("/test/input/path");
  int doc_version = 7;
  std::string doc_text_change = R"INPUT(
[Mesh]
  type = GeneratedMesh
  dim = 1
[]
[Executioner]
  type = Steady
[]
[Problem]
  solve = false
[]
  globalvar = ${fparse undefined}
)INPUT";

  // build didchange notification from parameters and handle it with server
  wasp::DataObject didchange_notification, diagnostics_notification;
  std::stringstream errors;
  EXPECT_TRUE(wasp::lsp::buildDidChangeNotification(
      didchange_notification, errors, doc_uri, doc_version, -1, -1, -1, -1, -1, doc_text_change));
  EXPECT_TRUE(
      moose_server->handleDidChangeNotification(didchange_notification, diagnostics_notification));

  // dissect diagnostics notification from server and create formatted list
  std::string response_uri;
  wasp::DataArray diagnostics_array;
  std::ostringstream diagnostics_list_actual;
  EXPECT_TRUE(wasp::lsp::dissectPublishDiagnosticsNotification(
      diagnostics_notification, errors, response_uri, diagnostics_array));
  format_diagnostics(diagnostics_array, diagnostics_list_actual);

  // check that diagnostics array size and message contents are as expected
  std::size_t diagnostics_size_expect = 1;
  std::string diagnostics_list_expect = R"INPUT(
line:11 column:2 -     no variable 'undefined' found for use in function parser expression
)INPUT";

  EXPECT_EQ(diagnostics_size_expect, diagnostics_array.size());
  EXPECT_EQ(diagnostics_list_expect, "\n" + diagnostics_list_actual.str());
}

TEST_F(MooseServerTest, DocumentCloseShutdownAndExit)
{
  // check moose_server can share connection it will use to read and write

  EXPECT_TRUE(moose_server->getConnection() != nullptr);

  // didclose test parameter

  std::string document_uri = wasp::lsp::m_uri_prefix + std::string("/test/input/path");

  // build didclose notification with the test parameter

  wasp::DataObject didclose_notification;
  std::stringstream didclose_errors;

  EXPECT_TRUE(
      wasp::lsp::buildDidCloseNotification(didclose_notification, didclose_errors, document_uri));

  EXPECT_TRUE(didclose_errors.str().empty());

  // handle the built didclose notification with the moose_server

  EXPECT_TRUE(moose_server->handleDidCloseNotification(didclose_notification));

  EXPECT_TRUE(moose_server->getErrors().empty());

  // shutdown test parameter

  int request_id = 27;

  // build shutdown request with the test parameters

  wasp::DataObject shutdown_request;
  std::stringstream shutdown_errors;

  EXPECT_TRUE(wasp::lsp::buildShutdownRequest(shutdown_request, shutdown_errors, request_id));

  EXPECT_TRUE(shutdown_errors.str().empty());

  // handle the built shutdown request with the moose_server

  wasp::DataObject shutdown_response;

  EXPECT_TRUE(moose_server->handleShutdownRequest(shutdown_request, shutdown_response));

  EXPECT_TRUE(moose_server->getErrors().empty());

  // check the dissected values of the moose_server shutdown response

  std::stringstream response_errors;
  int response_id;

  EXPECT_TRUE(wasp::lsp::dissectShutdownResponse(shutdown_response, response_errors, response_id));

  EXPECT_TRUE(response_errors.str().empty());

  EXPECT_EQ(request_id, response_id);

  // build exit notification which takes no extra parameters

  wasp::DataObject exit_notification;
  std::stringstream exit_errors;

  EXPECT_TRUE(wasp::lsp::buildExitNotification(exit_notification, exit_errors));

  EXPECT_TRUE(exit_errors.str().empty());

  // handle the built exit notification with the moose_server

  EXPECT_TRUE(moose_server->handleExitNotification(exit_notification));

  EXPECT_TRUE(moose_server->getErrors().empty());
}
