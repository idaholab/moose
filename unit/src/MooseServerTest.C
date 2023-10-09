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
#include "AppFactory.h"
#include "waspcore/Object.h"
#include "wasplsp/LSP.h"
#include "wasplsp/SymbolIterator.h"
#include <string>
#include <sstream>
#include <memory>
#include <vector>

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
      paths_stream << si.getPath() << "\n";
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
    struct CompletionInfo
    {
      std::string label;
      std::string new_text;
      std::string documentation;
      int start_line;
      int start_character;
      int end_line;
      int end_character;
    };

    std::vector<CompletionInfo> completions;
    std::size_t max_label = 0;
    std::size_t max_new_text = 0;
    std::size_t max_doc = 0;

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
                                                     completion_preselect));

      // truncate long descriptions and escape text newlines for easy viewing

      if (completion_documentation.length() > 50)
      {
        completion_documentation.resize(47);
        completion_documentation += "...";
      }

      MooseUtils::escape(completion_new_text);

      if (completion_label.size() > max_label)
        max_label = completion_label.size();

      if (completion_new_text.size() > max_new_text)
        max_new_text = completion_new_text.size();

      if (completion_documentation.size() > max_doc)
        max_doc = completion_documentation.size();

      completions.push_back({completion_label,
                             completion_new_text,
                             completion_documentation,
                             completion_start_line,
                             completion_start_character,
                             completion_end_line,
                             completion_end_character});
    }

    for (const auto & completion : completions)
    {
      completions_stream << "label: " << std::setw(max_label) << std::left << completion.label
                         << " text: " << std::setw(max_new_text) << std::left << completion.new_text
                         << " desc: " << std::setw(max_doc) << std::left << completion.documentation
                         << " pos: [" << completion.start_line << "." << completion.start_character
                         << "]-[" << completion.end_line << "." << completion.end_character << "]"
                         << "\n";
    }
  }

  // create moose_unit_app and moose_server to persist for reuse between tests
  static void SetUpTestCase()
  {
    moose_unit_app = AppFactory::createAppShared("MooseUnitApp", 0, nullptr);
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
  wasp::DataObject client_capabilities;

  // build initialize request with the test parameters

  wasp::DataObject initialize_request;
  std::stringstream initialize_errors;

  EXPECT_TRUE(wasp::lsp::buildInitializeRequest(initialize_request,
                                                initialize_errors,
                                                request_id,
                                                process_id,
                                                root_path,
                                                client_capabilities));

  EXPECT_TRUE(initialize_errors.str().empty());

  // handle the built initialize request with the moose_server

  wasp::DataObject initialize_response;

  EXPECT_TRUE(moose_server->handleInitializeRequest(initialize_request, initialize_response));

  EXPECT_TRUE(moose_server->getErrors().empty());

  // check the dissected values of the moose_server initialize response

  std::stringstream response_errors;
  int response_id;
  wasp::DataObject server_capabilities;

  EXPECT_TRUE(wasp::lsp::dissectInitializeResponse(
      initialize_response, response_errors, response_id, server_capabilities));

  EXPECT_TRUE(response_errors.str().empty());

  EXPECT_EQ(request_id, response_id);

  EXPECT_TRUE(server_capabilities[wasp::lsp::m_text_doc_sync][wasp::lsp::m_open_close].is_bool());
  EXPECT_TRUE(server_capabilities[wasp::lsp::m_text_doc_sync][wasp::lsp::m_open_close].to_bool());

  EXPECT_TRUE(server_capabilities[wasp::lsp::m_text_doc_sync][wasp::lsp::m_change].is_int());
  EXPECT_EQ(1, server_capabilities[wasp::lsp::m_text_doc_sync][wasp::lsp::m_change].to_int());

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

  EXPECT_TRUE(wasp::lsp::buildDidOpenNotification(didopen_notification,
                                                  didopen_errors,
                                                  document_uri,
                                                  document_language_id,
                                                  document_version,
                                                  document_text_open));

  EXPECT_TRUE(didopen_errors.str().empty());

  // force enabling performance logs to check reset by moose_server after call

  Moose::perf_log.enable_logging();

  // handle the built didopen notification with the moose_server

  wasp::DataObject diagnostics_notification;

  EXPECT_TRUE(
      moose_server->handleDidOpenNotification(didopen_notification, diagnostics_notification));

  EXPECT_TRUE(moose_server->getErrors().empty());

  // check behavior of performance logs was reset properly by the moose_server

  EXPECT_TRUE(Moose::perf_log.logging_enabled());

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
/Mesh (1:0)
/Mesh/[ (1:0)
/Mesh/decl (1:1)
/Mesh/] (1:5)
/Mesh/type (2:2)
/Mesh/type/decl (2:2)
/Mesh/type/= (2:7)
/Mesh/type/value (2:9)
/Mesh/dim (3:2)
/Mesh/dim/decl (3:2)
/Mesh/dim/= (3:6)
/Mesh/dim/value (3:8)
/Mesh/term (4:0)
/Variables (5:0)
/Variables/[ (5:0)
/Variables/decl (5:1)
/Variables/] (5:10)
/Variables/u (6:2)
/Variables/u/[ (6:2)
/Variables/u/decl (6:3)
/Variables/u/] (6:4)
/Variables/u/order (7:4)
/Variables/u/order/decl (7:4)
/Variables/u/order/= (7:10)
/Variables/u/order/value (7:12)
/Variables/u/family (8:4)
/Variables/u/family/decl (8:4)
/Variables/u/family/= (8:11)
/Variables/u/family/value (8:13)
/Variables/u/term (9:2)
/Variables/u (10:2)
/Variables/u/[ (10:2)
/Variables/u/decl (10:3)
/Variables/u/] (10:4)
/Variables/u/order (11:4)
/Variables/u/order/decl (11:4)
/Variables/u/order/= (11:10)
/Variables/u/order/value (11:12)
/Variables/u/family (12:4)
/Variables/u/family/decl (12:4)
/Variables/u/family/= (12:11)
/Variables/u/family/value (12:13)
/Variables/u/term (13:2)
/Variables/term (14:0)
/BCs (15:0)
/BCs/[ (15:0)
/BCs/decl (15:1)
/BCs/] (15:4)
/BCs/all (16:2)
/BCs/all/[ (16:2)
/BCs/all/decl (16:3)
/BCs/all/] (16:6)
/BCs/all/type (17:4)
/BCs/all/type/decl (17:4)
/BCs/all/type/= (17:9)
/BCs/all/type/value (17:11)
/BCs/all/boundary (18:4)
/BCs/all/boundary/decl (18:4)
/BCs/all/boundary/= (18:13)
/BCs/all/boundary/' (18:15)
/BCs/all/boundary/value (18:16)
/BCs/all/boundary/value (18:21)
/BCs/all/boundary/' (18:26)
/BCs/all/variable (19:4)
/BCs/all/variable/decl (19:4)
/BCs/all/variable/= (19:13)
/BCs/all/variable/value (19:15)
/BCs/all/term (20:2)
/BCs/term (21:0)
/Executioner (22:0)
/Executioner/[ (22:0)
/Executioner/decl (22:1)
/Executioner/] (22:12)
/Executioner/type (23:2)
/Executioner/type/decl (23:2)
/Executioner/type/= (23:7)
/Executioner/type/value (23:9)
/Executioner/term (24:0)
/Problem (25:0)
/Problem/[ (25:0)
/Problem/decl (25:1)
/Problem/] (25:8)
/Problem/solve (26:2)
/Problem/solve/decl (26:2)
/Problem/solve/= (26:8)
/Problem/solve/value (26:10)
/Problem/term (27:0)
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

  // force stopping performance logs to check reset by moose_server after call

  Moose::perf_log.disable_logging();

  // handle the built didchange notification with the moose_server

  wasp::DataObject diagnostics_notification;

  EXPECT_TRUE(
      moose_server->handleDidChangeNotification(didchange_notification, diagnostics_notification));

  EXPECT_TRUE(moose_server->getErrors().empty());

  // check behavior of performance logs was reset properly by the moose_server

  EXPECT_FALSE(Moose::perf_log.logging_enabled());

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
/Mesh (1:0)
/Mesh/[ (1:0)
/Mesh/decl (1:1)
/Mesh/] (1:5)
/Mesh/type (2:2)
/Mesh/type/decl (2:2)
/Mesh/type/= (2:7)
/Mesh/type/value (2:9)
/Mesh/dim (3:2)
/Mesh/dim/decl (3:2)
/Mesh/dim/= (3:6)
/Mesh/dim/value (3:8)
/Mesh/term (4:0)
/Variables (5:0)
/Variables/[ (5:0)
/Variables/decl (5:1)
/Variables/] (5:10)
/Variables/u (6:2)
/Variables/u/[ (6:2)
/Variables/u/decl (6:3)
/Variables/u/] (6:4)
/Variables/u/order (7:4)
/Variables/u/order/decl (7:4)
/Variables/u/order/= (7:10)
/Variables/u/order/value (7:12)
/Variables/u/family (8:4)
/Variables/u/family/decl (8:4)
/Variables/u/family/= (8:11)
/Variables/u/family/value (8:13)
/Variables/u/term (9:2)
/Variables/v (10:2)
/Variables/v/[ (10:2)
/Variables/v/decl (10:3)
/Variables/v/] (10:4)
/Variables/v/order (11:4)
/Variables/v/order/decl (11:4)
/Variables/v/order/= (11:10)
/Variables/v/order/value (11:12)
/Variables/v/family (12:4)
/Variables/v/family/decl (12:4)
/Variables/v/family/= (12:11)
/Variables/v/family/value (12:13)
/Variables/v/term (13:2)
/Variables/term (14:0)
/BCs (15:0)
/BCs/[ (15:0)
/BCs/decl (15:1)
/BCs/] (15:4)
/BCs/all (16:2)
/BCs/all/[ (16:2)
/BCs/all/decl (16:3)
/BCs/all/] (16:6)
/BCs/all/type (17:4)
/BCs/all/type/decl (17:4)
/BCs/all/type/= (17:9)
/BCs/all/type/value (17:11)
/BCs/all/boundary (18:4)
/BCs/all/boundary/decl (18:4)
/BCs/all/boundary/= (18:13)
/BCs/all/boundary/' (18:15)
/BCs/all/boundary/value (18:16)
/BCs/all/boundary/value (18:21)
/BCs/all/boundary/value (18:27)
/BCs/all/boundary/value (18:31)
/BCs/all/boundary/' (18:37)
/BCs/all/variable (19:4)
/BCs/all/variable/decl (19:4)
/BCs/all/variable/= (19:13)
/BCs/all/variable/value (19:15)
/BCs/all/term (20:2)
/BCs/term (21:0)
/Executioner (22:0)
/Executioner/[ (22:0)
/Executioner/decl (22:1)
/Executioner/] (22:12)
/Executioner/type (23:2)
/Executioner/type/decl (23:2)
/Executioner/type/= (23:7)
/Executioner/type/value (23:9)
/Executioner/term (24:0)
/Problem (25:0)
/Problem/[ (25:0)
/Problem/decl (25:1)
/Problem/] (25:8)
/Problem/solve (26:2)
/Problem/solve/decl (26:2)
/Problem/solve/= (26:8)
/Problem/solve/value (26:10)
/Problem/term (27:0)
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
  [disp_x]
  []
  [disp_y]
  []
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
    error_level = NONE
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
  int line = 6;
  int character = 0;

  // build completion request with the test parameters

  wasp::DataObject completion_request;
  std::stringstream completion_errors;

  EXPECT_TRUE(wasp::lsp::buildCompletionRequest(
      completion_request, completion_errors, request_id, doc_uri, line, character));

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

  EXPECT_EQ(request_id, response_id);

  EXPECT_EQ(47u, completions_array.size());

  std::ostringstream completions_actual;

  format_completions(completions_array, completions_actual);

  // expected completions with zero-based lines and columns

  std::string completions_expect = R"INPUT(
label: active                                 text: active = '__all__'                             desc:            If specified only the blocks named w... pos: [6.0]-[6.0]
label: add_subdomain_ids                      text: add_subdomain_ids = '0'                        desc:            The listed subdomains will be assume... pos: [6.0]-[6.0]
label: allow_renumbering                      text: allow_renumbering = true                       desc:            If allow_renumbering=false, node and... pos: [6.0]-[6.0]
label: alpha_rotation                         text: alpha_rotation = 0.0                           desc:            The number of degrees that the domai... pos: [6.0]-[6.0]
label: beta_rotation                          text: beta_rotation = 0.0                            desc:            The number of degrees that the domai... pos: [6.0]-[6.0]
label: block_id                               text: block_id = '0'                                 desc:            IDs of the block id/name pairs          pos: [6.0]-[6.0]
label: block_name                             text: block_name = 'value'                           desc:            Names of the block id/name pairs (mu... pos: [6.0]-[6.0]
label: boundary_id                            text: boundary_id = '0'                              desc:            IDs of the boundary id/name pairs       pos: [6.0]-[6.0]
label: boundary_name                          text: boundary_name = 'value'                        desc:            Names of the boundary id/name pairs ... pos: [6.0]-[6.0]
label: build_all_side_lowerd_mesh             text: build_all_side_lowerd_mesh = false             desc:            True to build the lower-dimensional ... pos: [6.0]-[6.0]
label: centroid_partitioner_direction         text: centroid_partitioner_direction = RADIAL        desc:            Specifies the sort direction if usin... pos: [6.0]-[6.0]
label: clear_spline_nodes                     text: clear_spline_nodes = false                     desc:            If clear_spline_nodes=true, IsoGeome... pos: [6.0]-[6.0]
label: construct_node_list_from_side_list     text: construct_node_list_from_side_list = true      desc:            Whether or not to generate nodesets ... pos: [6.0]-[6.0]
label: construct_side_list_from_node_list     text: construct_side_list_from_node_list = false     desc:            If true, construct side lists from t... pos: [6.0]-[6.0]
label: control_tags                           text: control_tags = 'value'                         desc:            Adds user-defined labels for accessi... pos: [6.0]-[6.0]
label: coord_block                            text: coord_block = 'value'                          desc:            Block IDs for the coordinate systems... pos: [6.0]-[6.0]
label: coord_type                             text: coord_type = 'XYZ'                             desc:            Type of the coordinate system per bl... pos: [6.0]-[6.0]
label: enable                                 text: enable = true                                  desc:            Set the enabled status of the MooseO... pos: [6.0]-[6.0]
label: file                                   text: file = value                                   desc: (REQUIRED) The name of the mesh file to read       pos: [6.0]-[6.0]
label: gamma_rotation                         text: gamma_rotation = 0.0                           desc:            The number of degrees that the domai... pos: [6.0]-[6.0]
label: ghosted_boundaries                     text: ghosted_boundaries = 'value'                   desc:            Boundaries to be ghosted if using Ne... pos: [6.0]-[6.0]
label: ghosted_boundaries_inflation           text: ghosted_boundaries_inflation = '0.0'           desc:            If you are using ghosted boundaries ... pos: [6.0]-[6.0]
label: ghosting_patch_size                    text: ghosting_patch_size = 0                        desc:            The number of nearest neighbors cons... pos: [6.0]-[6.0]
label: inactive                               text: inactive = 'value'                             desc:            If specified blocks matching these i... pos: [6.0]-[6.0]
label: include_local_in_ghosting              text: include_local_in_ghosting = false              desc:            Boolean used to toggle on the inclus... pos: [6.0]-[6.0]
label: length_unit                            text: length_unit = value                            desc:            How much distance one mesh length un... pos: [6.0]-[6.0]
label: max_leaf_size                          text: max_leaf_size = 10                             desc:            The maximum number of points in each... pos: [6.0]-[6.0]
label: nemesis                                text: nemesis = false                                desc:            If nemesis=true and file=foo.e, actu... pos: [6.0]-[6.0]
label: output_ghosting                        text: output_ghosting = false                        desc:            Boolean to turn on ghosting auxiliar... pos: [6.0]-[6.0]
label: partitioner                            text: partitioner = default                          desc:            Specifies a mesh partitioner to use ... pos: [6.0]-[6.0]
label: patch_size                             text: patch_size = 40                                desc:            The number of nodes to consider in t... pos: [6.0]-[6.0]
label: rz_coord_axis                          text: rz_coord_axis = Y                              desc:            The rotation axis (X | Y) for axisym... pos: [6.0]-[6.0]
label: rz_coord_blocks                        text: rz_coord_blocks = 'value'                      desc:            Blocks using general axisymmetric co... pos: [6.0]-[6.0]
label: rz_coord_directions                    text: rz_coord_directions = '0.0'                    desc:            Axis directions for each block in 'r... pos: [6.0]-[6.0]
label: rz_coord_origins                       text: rz_coord_origins = '0.0'                       desc:            Axis origin points for each block in... pos: [6.0]-[6.0]
label: second_order                           text: second_order = false                           desc:            Converts a first order mesh to a sec... pos: [6.0]-[6.0]
label: skip_deletion_repartition_after_refine text: skip_deletion_repartition_after_refine = false desc:            If the flag is true, uniform refinem... pos: [6.0]-[6.0]
label: skip_partitioning                      text: skip_partitioning = false                      desc:            If true the mesh won't be partitione... pos: [6.0]-[6.0]
label: skip_refine_when_use_split             text: skip_refine_when_use_split = true              desc:            True to skip uniform refinements whe... pos: [6.0]-[6.0]
label: split_file                             text: split_file = value                             desc:            Optional name of split mesh file(s) ... pos: [6.0]-[6.0]
label: type                                   text: type = FileMesh                                desc:            A string representing the Moose Obje... pos: [6.0]-[6.0]
label: uniform_refine                         text: uniform_refine = 0                             desc:            Specify the level of uniform refinem... pos: [6.0]-[6.0]
label: up_direction                           text: up_direction = X                               desc:            Specify what axis corresponds to the... pos: [6.0]-[6.0]
label: use_displaced_mesh                     text: use_displaced_mesh = true                      desc:            Create the displaced mesh if the 'di... pos: [6.0]-[6.0]
label: use_split                              text: use_split = false                              desc:            Use split distributed mesh files; is... pos: [6.0]-[6.0]
label: *                                      text: [block_name]\n  \n[]                           desc:            custom user named block                 pos: [6.0]-[6.0]
label: Partitioner                            text: [Partitioner]\n  \n[]                          desc:            application named block                 pos: [6.0]-[6.0]
)INPUT";

  EXPECT_EQ(completions_expect, "\n" + completions_actual.str());
}

TEST_F(MooseServerTest, CompletionDocumentRootLevel)
{
  // completion test parameters - at document root level outside of all blocks

  int request_id = 4;
  std::string doc_uri = wasp::lsp::m_uri_prefix + std::string("/test/input/path");
  int line = 42;
  int character = 0;

  // build completion request with the test parameters

  wasp::DataObject completion_request;
  std::stringstream completion_errors;

  EXPECT_TRUE(wasp::lsp::buildCompletionRequest(
      completion_request, completion_errors, request_id, doc_uri, line, character));

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

  EXPECT_EQ(request_id, response_id);

  EXPECT_EQ(45u, completions_array.size());

  std::ostringstream completions_actual;

  format_completions(completions_array, completions_actual);

  // expected completions with zero-based lines and columns

  std::string completions_expect = R"INPUT(
label: active               text: active = '__all__'             desc:            If specified only the blocks named w... pos: [42.0]-[42.0]
label: inactive             text: inactive = 'value'             desc:            If specified blocks matching these i... pos: [42.0]-[42.0]
label: Adaptivity           text: [Adaptivity]\n  \n[]           desc:            application named block                 pos: [42.0]-[42.0]
label: AuxKernels           text: [AuxKernels]\n  \n[]           desc:            application named block                 pos: [42.0]-[42.0]
label: AuxScalarKernels     text: [AuxScalarKernels]\n  \n[]     desc:            application named block                 pos: [42.0]-[42.0]
label: AuxVariables         text: [AuxVariables]\n  \n[]         desc:            application named block                 pos: [42.0]-[42.0]
label: BCs                  text: [BCs]\n  \n[]                  desc:            application named block                 pos: [42.0]-[42.0]
label: Bounds               text: [Bounds]\n  \n[]               desc:            application named block                 pos: [42.0]-[42.0]
label: Constraints          text: [Constraints]\n  \n[]          desc:            application named block                 pos: [42.0]-[42.0]
label: Controls             text: [Controls]\n  \n[]             desc:            application named block                 pos: [42.0]-[42.0]
label: DGKernels            text: [DGKernels]\n  \n[]            desc:            application named block                 pos: [42.0]-[42.0]
label: Dampers              text: [Dampers]\n  \n[]              desc:            application named block                 pos: [42.0]-[42.0]
label: Debug                text: [Debug]\n  \n[]                desc:            application named block                 pos: [42.0]-[42.0]
label: DeprecatedBlock      text: [DeprecatedBlock]\n  \n[]      desc:            application named block                 pos: [42.0]-[42.0]
label: DiracKernels         text: [DiracKernels]\n  \n[]         desc:            application named block                 pos: [42.0]-[42.0]
label: Distributions        text: [Distributions]\n  \n[]        desc:            application named block                 pos: [42.0]-[42.0]
label: Executioner          text: [Executioner]\n  \n[]          desc:            application named block                 pos: [42.0]-[42.0]
label: Executors            text: [Executors]\n  \n[]            desc:            application named block                 pos: [42.0]-[42.0]
label: FVBCs                text: [FVBCs]\n  \n[]                desc:            application named block                 pos: [42.0]-[42.0]
label: FVInterfaceKernels   text: [FVInterfaceKernels]\n  \n[]   desc:            application named block                 pos: [42.0]-[42.0]
label: FVKernels            text: [FVKernels]\n  \n[]            desc:            application named block                 pos: [42.0]-[42.0]
label: Functions            text: [Functions]\n  \n[]            desc:            application named block                 pos: [42.0]-[42.0]
label: FunctorMaterials     text: [FunctorMaterials]\n  \n[]     desc:            application named block                 pos: [42.0]-[42.0]
label: GlobalParams         text: [GlobalParams]\n  \n[]         desc:            application named block                 pos: [42.0]-[42.0]
label: ICs                  text: [ICs]\n  \n[]                  desc:            application named block                 pos: [42.0]-[42.0]
label: InterfaceKernels     text: [InterfaceKernels]\n  \n[]     desc:            application named block                 pos: [42.0]-[42.0]
label: Kernels              text: [Kernels]\n  \n[]              desc:            application named block                 pos: [42.0]-[42.0]
label: Materials            text: [Materials]\n  \n[]            desc:            application named block                 pos: [42.0]-[42.0]
label: Mesh                 text: [Mesh]\n  \n[]                 desc:            application named block                 pos: [42.0]-[42.0]
label: MultiApps            text: [MultiApps]\n  \n[]            desc:            application named block                 pos: [42.0]-[42.0]
label: NodalKernels         text: [NodalKernels]\n  \n[]         desc:            application named block                 pos: [42.0]-[42.0]
label: NodalNormals         text: [NodalNormals]\n  \n[]         desc:            application named block                 pos: [42.0]-[42.0]
label: Outputs              text: [Outputs]\n  \n[]              desc:            application named block                 pos: [42.0]-[42.0]
label: Positions            text: [Positions]\n  \n[]            desc:            application named block                 pos: [42.0]-[42.0]
label: Postprocessors       text: [Postprocessors]\n  \n[]       desc:            application named block                 pos: [42.0]-[42.0]
label: Preconditioning      text: [Preconditioning]\n  \n[]      desc:            application named block                 pos: [42.0]-[42.0]
label: Problem              text: [Problem]\n  \n[]              desc:            application named block                 pos: [42.0]-[42.0]
label: Reporters            text: [Reporters]\n  \n[]            desc:            application named block                 pos: [42.0]-[42.0]
label: Samplers             text: [Samplers]\n  \n[]             desc:            application named block                 pos: [42.0]-[42.0]
label: ScalarKernels        text: [ScalarKernels]\n  \n[]        desc:            application named block                 pos: [42.0]-[42.0]
label: Times                text: [Times]\n  \n[]                desc:            application named block                 pos: [42.0]-[42.0]
label: Transfers            text: [Transfers]\n  \n[]            desc:            application named block                 pos: [42.0]-[42.0]
label: UserObjects          text: [UserObjects]\n  \n[]          desc:            application named block                 pos: [42.0]-[42.0]
label: Variables            text: [Variables]\n  \n[]            desc:            application named block                 pos: [42.0]-[42.0]
label: VectorPostprocessors text: [VectorPostprocessors]\n  \n[] desc:            application named block                 pos: [42.0]-[42.0]
)INPUT";

  EXPECT_EQ(completions_expect, "\n" + completions_actual.str());
}

TEST_F(MooseServerTest, CompletionValueActiveBlocks)
{
  // completion test parameters - on active parameter value in Variables block

  int request_id = 4;
  std::string doc_uri = wasp::lsp::m_uri_prefix + std::string("/test/input/path");
  int line = 9;
  int character = 12;

  // build completion request with the test parameters

  wasp::DataObject completion_request;
  std::stringstream completion_errors;

  EXPECT_TRUE(wasp::lsp::buildCompletionRequest(
      completion_request, completion_errors, request_id, doc_uri, line, character));

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

  EXPECT_EQ(request_id, response_id);

  EXPECT_EQ(2u, completions_array.size());

  std::ostringstream completions_actual;

  format_completions(completions_array, completions_actual);

  // expected completions with zero-based lines and columns

  std::string completions_expect = R"INPUT(
label: u text: u desc: subblock name pos: [9.12]-[9.19]
label: v text: v desc: subblock name pos: [9.12]-[9.19]
)INPUT";

  EXPECT_EQ(completions_expect, "\n" + completions_actual.str());
}

TEST_F(MooseServerTest, CompletionValueBooleanParam)
{
  // completion test parameters - on boolean value of solve param from Problem

  int request_id = 4;
  std::string doc_uri = wasp::lsp::m_uri_prefix + std::string("/test/input/path");
  int line = 33;
  int character = 10;

  // build completion request with the test parameters

  wasp::DataObject completion_request;
  std::stringstream completion_errors;

  EXPECT_TRUE(wasp::lsp::buildCompletionRequest(
      completion_request, completion_errors, request_id, doc_uri, line, character));

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

  EXPECT_EQ(request_id, response_id);

  EXPECT_EQ(2u, completions_array.size());

  std::ostringstream completions_actual;

  format_completions(completions_array, completions_actual);

  // expected completions with zero-based lines and columns

  std::string completions_expect = R"INPUT(
label: false text: false desc:  pos: [33.10]-[33.15]
label: true  text: true  desc:  pos: [33.10]-[33.15]
)INPUT";

  EXPECT_EQ(completions_expect, "\n" + completions_actual.str());
}

TEST_F(MooseServerTest, CompletionValueEnumsAndDocs)
{
  // completion test parameters - on error_level enum in Terminator UserObject

  int request_id = 4;
  std::string doc_uri = wasp::lsp::m_uri_prefix + std::string("/test/input/path");
  int line = 39;
  int character = 18;

  // build completion request with the test parameters

  wasp::DataObject completion_request;
  std::stringstream completion_errors;

  EXPECT_TRUE(wasp::lsp::buildCompletionRequest(
      completion_request, completion_errors, request_id, doc_uri, line, character));

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

  EXPECT_EQ(request_id, response_id);

  EXPECT_EQ(4u, completions_array.size());

  std::ostringstream completions_actual;

  format_completions(completions_array, completions_actual);

  // expected completions with zero-based lines and columns

  std::string completions_expect = R"INPUT(
label: ERROR   text: ERROR   desc: Throw a MOOSE error, resulting in the terminati... pos: [39.18]-[39.22]
label: INFO    text: INFO    desc: Output an information message once.                pos: [39.18]-[39.22]
label: NONE    text: NONE    desc: No message will be printed.                        pos: [39.18]-[39.22]
label: WARNING text: WARNING desc: Output a warning message once.                     pos: [39.18]-[39.22]
)INPUT";

  EXPECT_EQ(completions_expect, "\n" + completions_actual.str());
}

TEST_F(MooseServerTest, CompletionValueAllowedTypes)
{
  // completion test parameters - on type parameter value in Executioner block

  int request_id = 4;
  std::string doc_uri = wasp::lsp::m_uri_prefix + std::string("/test/input/path");
  int line = 30;
  int character = 9;

  // build completion request with the test parameters

  wasp::DataObject completion_request;
  std::stringstream completion_errors;

  EXPECT_TRUE(wasp::lsp::buildCompletionRequest(
      completion_request, completion_errors, request_id, doc_uri, line, character));

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

  EXPECT_EQ(request_id, response_id);

  EXPECT_EQ(5u, completions_array.size());

  std::ostringstream completions_actual;

  format_completions(completions_array, completions_actual);

  // expected completions with zero-based lines and columns

  std::string completions_expect = R"INPUT(
label: Eigenvalue         text: Eigenvalue         desc: Eigenvalue solves a standard/generalized linear... pos: [30.9]-[30.18]
label: InversePowerMethod text: InversePowerMethod desc: Inverse power method for eigenvalue problems.      pos: [30.9]-[30.18]
label: NonlinearEigen     text: NonlinearEigen     desc: Executioner for eigenvalue problems.               pos: [30.9]-[30.18]
label: Steady             text: Steady             desc: Executioner for steady-state simulations.          pos: [30.9]-[30.18]
label: Transient          text: Transient          desc: Executioner for time varying simulations.          pos: [30.9]-[30.18]
)INPUT";

  EXPECT_EQ(completions_expect, "\n" + completions_actual.str());
}

TEST_F(MooseServerTest, CompletionValueInputLookups)
{
  // completion test parameters - on displacements parameter value in VacuumBC

  int request_id = 4;
  std::string doc_uri = wasp::lsp::m_uri_prefix + std::string("/test/input/path");
  int line = 26;
  int character = 21;

  // build completion request with the test parameters

  wasp::DataObject completion_request;
  std::stringstream completion_errors;

  EXPECT_TRUE(wasp::lsp::buildCompletionRequest(
      completion_request, completion_errors, request_id, doc_uri, line, character));

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

  EXPECT_EQ(request_id, response_id);

  EXPECT_EQ(4u, completions_array.size());

  std::ostringstream completions_actual;

  format_completions(completions_array, completions_actual);

  // expected completions with zero-based lines and columns

  std::string completions_expect = R"INPUT(
label: disp_x text: disp_x desc: from /AuxVariables/* pos: [26.21]-[26.27]
label: disp_y text: disp_y desc: from /AuxVariables/* pos: [26.21]-[26.27]
label: u      text: u      desc: from /Variables/*    pos: [26.21]-[26.27]
label: v      text: v      desc: from /Variables/*    pos: [26.21]-[26.27]
)INPUT";

  EXPECT_EQ(completions_expect, "\n" + completions_actual.str());
}

TEST_F(MooseServerTest, DocumentDefinitionRequest)
{
  // definition test parameters

  int request_id = 5;
  std::string document_uri = wasp::lsp::m_uri_prefix + std::string("/test/input/path");
  int line = 0;
  int character = 0;

  // build definition request with the test parameters

  wasp::DataObject definition_request;
  std::stringstream definition_errors;

  EXPECT_TRUE(wasp::lsp::buildDefinitionRequest(
      definition_request, definition_errors, request_id, document_uri, line, character));

  EXPECT_TRUE(definition_errors.str().empty());

  // handle the built definition request with the moose_server

  wasp::DataObject definition_response;

  EXPECT_TRUE(moose_server->handleDefinitionRequest(definition_request, definition_response));

  EXPECT_TRUE(moose_server->getErrors().empty());

  // definition response will be checked when capability is implemented
}

TEST_F(MooseServerTest, DocumentReferencesRequest)
{
  // references test parameters

  int request_id = 6;
  std::string document_uri = wasp::lsp::m_uri_prefix + std::string("/test/input/path");
  int line = 0;
  int character = 0;
  bool include_declaration = false;

  // build references request with the test parameters

  wasp::DataObject references_request;
  std::stringstream references_errors;

  EXPECT_TRUE(wasp::lsp::buildReferencesRequest(references_request,
                                                references_errors,
                                                request_id,
                                                document_uri,
                                                line,
                                                character,
                                                include_declaration));

  EXPECT_TRUE(references_errors.str().empty());

  // handle the built references request with the moose_server

  wasp::DataObject references_response;

  EXPECT_TRUE(moose_server->handleReferencesRequest(references_request, references_response));

  EXPECT_TRUE(moose_server->getErrors().empty());

  // references response will be checked when capability is implemented
}

TEST_F(MooseServerTest, DocumentFormattingRequest)
{
  // formatting test parameters

  int request_id = 7;
  std::string document_uri = wasp::lsp::m_uri_prefix + std::string("/test/input/path");
  int tab_size = 4;
  int insert_spaces = true;

  // build formatting request with the test parameters

  wasp::DataObject formatting_request;
  std::stringstream formatting_errors;

  EXPECT_TRUE(wasp::lsp::buildFormattingRequest(
      formatting_request, formatting_errors, request_id, document_uri, tab_size, insert_spaces));

  EXPECT_TRUE(formatting_errors.str().empty());

  // handle the built formatting request with the moose_server

  wasp::DataObject formatting_response;

  EXPECT_TRUE(moose_server->handleFormattingRequest(formatting_request, formatting_response));

  EXPECT_TRUE(moose_server->getErrors().empty());

  // formatting response will be checked when capability is implemented
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

  int request_id = 8;

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
