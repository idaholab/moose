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
                          std::stringstream & diagnostics_stream) const
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

      ASSERT_TRUE(wasp::lsp::dissectDiagnosticObject(*(diagnostics_array.at(i).to_object()),
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
                         << std::endl;
    }
  }

  // traverse symbolpaths and make formatted list for easy testing and viewing
  void format_symbolpaths(const wasp::DataObject & symbols_response,
                          std::stringstream & paths_stream) const
  {
    ASSERT_TRUE(wasp::lsp::verifySymbolsResponse(symbols_response));
    wasp::lsp::SymbolIterator si(std::make_shared<wasp::DataObject>(symbols_response));

    for (std::vector<int> indices{0}; indices.back() < (int)si.getChildSize(); indices.back()++)
    {
      si.moveToChildAt(indices.back());
      indices.push_back(-1);
      paths_stream << si.getPath() << std::endl;
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

TEST_F(MooseServerTest, InitializeCapabilities)
{
  // initialize test parameters

  int request_id = 1;
  int process_id = -1;
  std::string root_path = "";
  wasp::DataObject client_capabilities;

  // build initialize request with the test parameters

  wasp::DataObject initialize_request;
  std::stringstream initialize_errors;

  ASSERT_TRUE(wasp::lsp::buildInitializeRequest(initialize_request,
                                                initialize_errors,
                                                request_id,
                                                process_id,
                                                root_path,
                                                client_capabilities));

  ASSERT_TRUE(initialize_errors.str().empty());

  // handle the built initialize request with the moose_server

  wasp::DataObject initialize_response;

  ASSERT_TRUE(moose_server->handleInitializeRequest(initialize_request, initialize_response));

  ASSERT_TRUE(moose_server->getErrors().empty());

  // check the dissected values of the moose_server initialize response

  std::stringstream response_errors;
  int response_id;
  wasp::DataObject server_capabilities;

  ASSERT_TRUE(wasp::lsp::dissectInitializeResponse(
      initialize_response, response_errors, response_id, server_capabilities));

  ASSERT_TRUE(response_errors.str().empty());

  ASSERT_EQ(request_id, response_id);

  ASSERT_TRUE(server_capabilities[wasp::lsp::m_text_doc_sync][wasp::lsp::m_open_close].is_bool());
  ASSERT_TRUE(server_capabilities[wasp::lsp::m_text_doc_sync][wasp::lsp::m_open_close].to_bool());

  ASSERT_TRUE(server_capabilities[wasp::lsp::m_text_doc_sync][wasp::lsp::m_change].is_int());
  ASSERT_EQ(1, server_capabilities[wasp::lsp::m_text_doc_sync][wasp::lsp::m_change].to_int());
}

TEST_F(MooseServerTest, DocumentOpenDiagnostics)
{
  // didopen test parameters - note input has error with duplicate meshes

  std::string document_uri = "/test/document/uri/string";
  std::string document_language_id = "test_language_id_string";
  int document_version = 1;
  std::string document_text_open = R"INPUT(
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]
[Executioner]
  type = Transient
  num_steps = 1
  dt = 1
[]
[Problem]
  solve = false
[]
[Outputs]
  console = false
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
      moose_server->handleDidOpenNotification(didopen_notification, diagnostics_notification));

  ASSERT_TRUE(moose_server->getErrors().empty());

  // check set of errors built from the moose_server diagnostics notification

  std::stringstream diagnostics_errors;
  std::string response_uri;
  wasp::DataArray diagnostics_array;

  ASSERT_TRUE(wasp::lsp::dissectPublishDiagnosticsNotification(
      diagnostics_notification, diagnostics_errors, response_uri, diagnostics_array));

  ASSERT_TRUE(diagnostics_errors.str().empty());

  ASSERT_EQ(document_uri, response_uri);

  ASSERT_EQ(6, (int)diagnostics_array.size());

  std::stringstream diagnostics_actual;

  format_diagnostics(diagnostics_array, diagnostics_actual);

  // expected diagnostics with zero-based lines and columns - duplicate meshes

  std::string diagnostics_expect = R"INPUT(
line:2 column:2 - parameter 'Mesh/type' supplied multiple times
line:7 column:2 - parameter 'Mesh/type' supplied multiple times
line:3 column:2 - parameter 'Mesh/dim' supplied multiple times
line:8 column:2 - parameter 'Mesh/dim' supplied multiple times
line:4 column:2 - parameter 'Mesh/nx' supplied multiple times
line:9 column:2 - parameter 'Mesh/nx' supplied multiple times
)INPUT";

  ASSERT_EQ(diagnostics_expect, "\n" + diagnostics_actual.str());
}

TEST_F(MooseServerTest, DocumentOpenSymbols)
{
  // symbols test parameters

  int request_id = 2;
  std::string document_uri = "/test/document/uri/string";

  // build symbols request with the test parameters

  wasp::DataObject symbols_request;
  std::stringstream symbols_errors;

  ASSERT_TRUE(
      wasp::lsp::buildSymbolsRequest(symbols_request, symbols_errors, request_id, document_uri));

  ASSERT_TRUE(symbols_errors.str().empty());

  // handle the built symbols request with the moose_server

  wasp::DataObject symbols_response;

  ASSERT_TRUE(moose_server->handleSymbolsRequest(symbols_request, symbols_response));

  ASSERT_TRUE(moose_server->getErrors().empty());

  // check set of paths built from the moose_server symbols response

  std::stringstream paths_actual;

  format_symbolpaths(symbols_response, paths_actual);

  // expected paths with zero-based lines and columns - note duplicate meshes

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
/Mesh/nx (4:2)
/Mesh/nx/decl (4:2)
/Mesh/nx/= (4:5)
/Mesh/nx/value (4:7)
/Mesh/term (5:0)
/Mesh (6:0)
/Mesh/[ (6:0)
/Mesh/decl (6:1)
/Mesh/] (6:5)
/Mesh/type (7:2)
/Mesh/type/decl (7:2)
/Mesh/type/= (7:7)
/Mesh/type/value (7:9)
/Mesh/dim (8:2)
/Mesh/dim/decl (8:2)
/Mesh/dim/= (8:6)
/Mesh/dim/value (8:8)
/Mesh/nx (9:2)
/Mesh/nx/decl (9:2)
/Mesh/nx/= (9:5)
/Mesh/nx/value (9:7)
/Mesh/term (10:0)
/Executioner (11:0)
/Executioner/[ (11:0)
/Executioner/decl (11:1)
/Executioner/] (11:12)
/Executioner/type (12:2)
/Executioner/type/decl (12:2)
/Executioner/type/= (12:7)
/Executioner/type/value (12:9)
/Executioner/num_steps (13:2)
/Executioner/num_steps/decl (13:2)
/Executioner/num_steps/= (13:12)
/Executioner/num_steps/value (13:14)
/Executioner/dt (14:2)
/Executioner/dt/decl (14:2)
/Executioner/dt/= (14:5)
/Executioner/dt/value (14:7)
/Executioner/term (15:0)
/Problem (16:0)
/Problem/[ (16:0)
/Problem/decl (16:1)
/Problem/] (16:8)
/Problem/solve (17:2)
/Problem/solve/decl (17:2)
/Problem/solve/= (17:8)
/Problem/solve/value (17:10)
/Problem/term (18:0)
/Outputs (19:0)
/Outputs/[ (19:0)
/Outputs/decl (19:1)
/Outputs/] (19:8)
/Outputs/console (20:2)
/Outputs/console/decl (20:2)
/Outputs/console/= (20:10)
/Outputs/console/value (20:12)
/Outputs/term (21:0)
)INPUT";

  ASSERT_EQ(paths_expect, "\n" + paths_actual.str());
}

TEST_F(MooseServerTest, DocumentChangeDiagnostics)
{
  // didchange test parameters - note input has error with Executioner/typo

  std::string document_uri = "/test/document/uri/string";
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
  nx = 1
[]
[Executioner]
  type = Transient
  num_steps = 1
  typo = 1
[]
[Problem]
  solve = false
[]
[Outputs]
  console = false
[]
)INPUT";

  // build didchange notification with the test parameters

  wasp::DataObject didchange_notification;
  std::stringstream didchange_errors;

  ASSERT_TRUE(wasp::lsp::buildDidChangeNotification(didchange_notification,
                                                    didchange_errors,
                                                    document_uri,
                                                    document_version,
                                                    start_line,
                                                    start_character,
                                                    end_line,
                                                    end_character,
                                                    range_length,
                                                    document_text_change));

  ASSERT_TRUE(didchange_errors.str().empty());

  // handle the built didchange notification with the moose_server

  wasp::DataObject diagnostics_notification;

  ASSERT_TRUE(
      moose_server->handleDidChangeNotification(didchange_notification, diagnostics_notification));

  ASSERT_TRUE(moose_server->getErrors().empty());

  // check set of errors built from the moose_server diagnostics notification

  std::stringstream diagnostics_errors;
  std::string response_uri;
  wasp::DataArray diagnostics_array;

  ASSERT_TRUE(wasp::lsp::dissectPublishDiagnosticsNotification(
      diagnostics_notification, diagnostics_errors, response_uri, diagnostics_array));

  ASSERT_TRUE(diagnostics_errors.str().empty());

  ASSERT_EQ(document_uri, response_uri);

  ASSERT_EQ(2, (int)diagnostics_array.size());

  std::stringstream diagnostics_actual;

  format_diagnostics(diagnostics_array, diagnostics_actual);

  // expected diagnostics with zero-based lines and columns - Executioner/typo

  std::string diagnostics_expect = R"INPUT(
line:9 column:2 - unused parameter 'Executioner/typo'
line:9 column:2 -       Did you mean 'type'?
)INPUT";

  ASSERT_EQ(diagnostics_expect, "\n" + diagnostics_actual.str());
}

TEST_F(MooseServerTest, DocumentChangeSymbols)
{
  // symbols test parameters

  int request_id = 1;
  std::string document_uri = "/test/document/uri/string";

  // build symbols request with the test parameters

  wasp::DataObject symbols_request;
  std::stringstream symbols_errors;

  ASSERT_TRUE(
      wasp::lsp::buildSymbolsRequest(symbols_request, symbols_errors, request_id, document_uri));

  ASSERT_TRUE(symbols_errors.str().empty());

  // handle the built symbols request with the moose_server

  wasp::DataObject symbols_response;

  ASSERT_TRUE(moose_server->handleSymbolsRequest(symbols_request, symbols_response));

  ASSERT_TRUE(moose_server->getErrors().empty());

  // check set of paths built from the moose_server symbols response

  std::stringstream paths_actual;

  format_symbolpaths(symbols_response, paths_actual);

  // expected paths with zero-based lines and columns - note Executioner/typo

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
/Mesh/nx (4:2)
/Mesh/nx/decl (4:2)
/Mesh/nx/= (4:5)
/Mesh/nx/value (4:7)
/Mesh/term (5:0)
/Executioner (6:0)
/Executioner/[ (6:0)
/Executioner/decl (6:1)
/Executioner/] (6:12)
/Executioner/type (7:2)
/Executioner/type/decl (7:2)
/Executioner/type/= (7:7)
/Executioner/type/value (7:9)
/Executioner/num_steps (8:2)
/Executioner/num_steps/decl (8:2)
/Executioner/num_steps/= (8:12)
/Executioner/num_steps/value (8:14)
/Executioner/typo (9:2)
/Executioner/typo/decl (9:2)
/Executioner/typo/= (9:7)
/Executioner/typo/value (9:9)
/Executioner/term (10:0)
/Problem (11:0)
/Problem/[ (11:0)
/Problem/decl (11:1)
/Problem/] (11:8)
/Problem/solve (12:2)
/Problem/solve/decl (12:2)
/Problem/solve/= (12:8)
/Problem/solve/value (12:10)
/Problem/term (13:0)
/Outputs (14:0)
/Outputs/[ (14:0)
/Outputs/decl (14:1)
/Outputs/] (14:8)
/Outputs/console (15:2)
/Outputs/console/decl (15:2)
/Outputs/console/= (15:10)
/Outputs/console/value (15:12)
/Outputs/term (16:0)
)INPUT";

  ASSERT_EQ(paths_expect, "\n" + paths_actual.str());
}

#endif
