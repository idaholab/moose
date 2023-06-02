//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef WASP_ENABLED

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
    size_t diagnostics_size = diagnostics_array.size();

    for (size_t i = 0; i < diagnostics_size; i++)
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

  EXPECT_EQ(4, (int)diagnostics_array.size());

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

  EXPECT_EQ(7, (int)diagnostics_array.size());

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

TEST_F(MooseServerTest, DocumentCompletionRequest)
{
  // completion test parameters

  int request_id = 4;
  std::string document_uri = wasp::lsp::m_uri_prefix + std::string("/test/input/path");
  int line = 0;
  int character = 0;

  // build completion request with the test parameters

  wasp::DataObject completion_request;
  std::stringstream completion_errors;

  EXPECT_TRUE(wasp::lsp::buildCompletionRequest(
      completion_request, completion_errors, request_id, document_uri, line, character));

  EXPECT_TRUE(completion_errors.str().empty());

  // handle the built completion request with the moose_server

  wasp::DataObject completion_response;

  EXPECT_TRUE(moose_server->handleCompletionRequest(completion_request, completion_response));

  EXPECT_TRUE(moose_server->getErrors().empty());

  // completion response will be checked when capability is implemented
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
  int start_line = 0;
  int start_character = 0;
  int end_line = 0;
  int end_character = 0;
  int tab_size = 4;
  int insert_spaces = true;

  // build formatting request with the test parameters

  wasp::DataObject formatting_request;
  std::stringstream formatting_errors;

  EXPECT_TRUE(wasp::lsp::buildFormattingRequest(formatting_request,
                                                formatting_errors,
                                                request_id,
                                                document_uri,
                                                start_line,
                                                start_character,
                                                end_line,
                                                end_character,
                                                tab_size,
                                                insert_spaces));

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

#endif
