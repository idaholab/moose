//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef WASP_ENABLED

#include "MooseServer.h"
#include "Moose.h"
#include "InputParameters.h"
#include "AppFactory.h"
#include "pcrecpp.h"
#include <cstdio>

bool
MooseServer::setup(MooseApp * moose_app)
{
  // cache a pointer to the application that set up this server instance

  _moose_app = moose_app;

  _is_setup = true;

  return true;
}

bool
MooseServer::parseDocumentForDiagnostics(wasp::DataArray & diagnosticsList)
{
  if (!_is_setup)
  {
    errors << wasp::lsp::m_error_prefix << "Server needs to be setup" << std::endl;
    return false;
  }

  if (!is_initialized)
  {
    errors << wasp::lsp::m_error_prefix << "Server needs to be initialized" << std::endl;
    return false;
  }

  if (!is_document_open)
  {
    errors << wasp::lsp::m_error_prefix << "Server has no open document" << std::endl;
    return false;
  }

  // strip prefix from document path if it exists and append file suffix

  std::string parse_file_path = document_path + ".tmpls";

  if (parse_file_path.rfind(wasp::lsp::m_uri_prefix, 0) == 0)
  {
    parse_file_path.erase(0, std::string(wasp::lsp::m_uri_prefix).size());
  }

  // write file content to temporary path that an application will parse

  std::ofstream parse_file_stream(parse_file_path.c_str());

  if (parse_file_stream.fail())
  {
    mooseError("Unable to open file ", parse_file_path);
  }

  parse_file_stream << document_text;

  parse_file_stream.close();

  // copy parent application parameters and modify to set up input check

  InputParameters app_params = _moose_app->parameters();

  app_params.set<std::vector<std::string>>("input_file") = {parse_file_path};
  app_params.set<bool>("check_input") = true;
  app_params.set<bool>("error_unused") = true;
  app_params.set<bool>("error") = true;
  app_params.set<std::string>("color") = "off";
  app_params.set<bool>("disable_perf_graph_live") = true;
  app_params.set<bool>("language_server") = true;

  // create new application with parameters modified for input check run

  _check_app = AppFactory::instance().createShared(
      _moose_app->type(), _moose_app->name(), app_params, _moose_app->getCommunicator()->get());

  // disable logs and enable error exceptions with initial values cached

  bool cached_logging_enabled = Moose::perf_log.logging_enabled();

  Moose::perf_log.disable_logging();

  bool cached_throw_on_error = Moose::_throw_on_error;

  Moose::_throw_on_error = true;

  bool pass = true;

  // run input check application converting caught errors to diagnostics

  try
  {
    _check_app->run();
  }
  catch (std::exception & err)
  {
    int line_number = 1;
    int column_number = 1;

    // walk over caught message line by line adding each as a diagnostic

    std::istringstream caught_msg(err.what());

    for (std::string error_line; std::getline(caught_msg, error_line);)
    {
      // skip over lines that are blank or contain only the error prefix

      if (error_line.empty() || error_line == "*** ERROR ***")
        continue;

      // check if this error line already has the input file path prefix

      if (error_line.rfind(parse_file_path + ":", 0) == 0)
      {
        // strip input file path and colon prefix off of this error line

        error_line.erase(0, parse_file_path.size() + 1);

        int match_line_number;
        int match_column_number;
        std::string match_error_line;

        // get line and column number from this error line if both exist

        if (pcrecpp::RE("^(\\d+)\\.(\\d+)\\-?\\d*:\\s*(.*)$")
                .FullMatch(error_line, &match_line_number, &match_column_number, &match_error_line))
        {
          line_number = match_line_number;
          column_number = match_column_number;
          error_line = match_error_line;
        }

        // otherwise get line number off of this error line if it exists

        else if (pcrecpp::RE("^(\\d+):\\s*(.*)$")
                     .FullMatch(error_line, &match_line_number, &match_error_line))
        {
          line_number = match_line_number;
          column_number = 1;
          error_line = match_error_line;
        }
      }

      // build diagnostic object from the error line and add to the list

      diagnosticsList.push_back(wasp::DataObject());

      wasp::DataObject * diagnostic = diagnosticsList.back().to_object();

      // according to the protocol line and column number are zero based

      pass &= wasp::lsp::buildDiagnosticObject(*diagnostic,
                                               errors,
                                               line_number - 1,
                                               column_number - 1,
                                               line_number - 1,
                                               column_number - 1,
                                               1,
                                               "moose_srv",
                                               "check_inp",
                                               error_line);
    }
  }

  // reset behaviors of performance logging and error exception throwing

  if (cached_logging_enabled)
    Moose::perf_log.enable_logging();

  Moose::_throw_on_error = cached_throw_on_error;

  // remove temporary file that was dumped to the disk for parser access

  std::remove(parse_file_path.c_str());

  return pass;
}

bool
MooseServer::updateDocumentTextChanges(const std::string & replacement_text,
                                       int start_line,
                                       int start_character,
                                       int end_line,
                                       int end_character,
                                       int range_length)
{
  if (!_is_setup)
  {
    errors << wasp::lsp::m_error_prefix << "Server needs to be setup" << std::endl;
    return false;
  }

  if (!is_initialized)
  {
    errors << wasp::lsp::m_error_prefix << "Server needs to be initialized" << std::endl;
    return false;
  }

  if (!is_document_open)
  {
    errors << wasp::lsp::m_error_prefix << "Server has no open document" << std::endl;
    return false;
  }

  // these are the expected values to indicate full document replacement

  if (!(start_line == -1 && start_character == -1 && end_line == -1 && end_character == -1 &&
        range_length == -1))
  {
    errors << wasp::lsp::m_error_prefix << "Server needs full text when changed" << std::endl;
    return false;
  }

  document_text = replacement_text;

  return true;
}

bool
MooseServer::gatherDocumentCompletionItems(wasp::DataArray & /* completionItems */,
                                           bool & /* is_incomplete */,
                                           int /* line */,
                                           int /* character */)
{

  if (!_is_setup)
  {
    errors << wasp::lsp::m_error_prefix << "Server needs to be setup" << std::endl;
    return false;
  }

  if (!is_initialized)
  {
    errors << wasp::lsp::m_error_prefix << "Server needs to be initialized" << std::endl;
    return false;
  }

  if (!is_document_open)
  {
    errors << wasp::lsp::m_error_prefix << "Server has no open document" << std::endl;
    return false;
  }

  bool pass = true;

  // TODO - hook up this capability by adding server specific logic here

  return pass;
}

bool
MooseServer::gatherDocumentDefinitionLocations(wasp::DataArray & /* definitionLocations */,
                                               int /* line */,
                                               int /* character */)
{
  if (!_is_setup)
  {
    errors << wasp::lsp::m_error_prefix << "Server needs to be setup" << std::endl;
    return false;
  }

  if (!is_initialized)
  {
    errors << wasp::lsp::m_error_prefix << "Server needs to be initialized" << std::endl;
    return false;
  }

  if (!is_document_open)
  {
    errors << wasp::lsp::m_error_prefix << "Server has no open document" << std::endl;
    return false;
  }

  bool pass = true;

  // TODO - hook up this capability by adding server specific logic here

  return pass;
}

bool
MooseServer::gatherDocumentReferencesLocations(wasp::DataArray & /* referencesLocations */,
                                               int /* line */,
                                               int /* character */,
                                               bool /* include_declaration */)
{
  if (!_is_setup)
  {
    errors << wasp::lsp::m_error_prefix << "Server needs to be setup" << std::endl;
    return false;
  }

  if (!is_initialized)
  {
    errors << wasp::lsp::m_error_prefix << "Server needs to be initialized" << std::endl;
    return false;
  }

  if (!is_document_open)
  {
    errors << wasp::lsp::m_error_prefix << "Server has no open document" << std::endl;
    return false;
  }

  bool pass = true;

  // TODO - hook up this capability by adding server specific logic here

  return pass;
}

bool
MooseServer::gatherDocumentFormattingTextEdits(wasp::DataArray & /* formattingTextEdits */,
                                               int /* start_line */,
                                               int /* start_character */,
                                               int /* end_line */,
                                               int /* end_character */,
                                               int /* tab_size */,
                                               bool /* insert_spaces */)
{
  if (!_is_setup)
  {
    errors << wasp::lsp::m_error_prefix << "Server needs to be setup" << std::endl;
    return false;
  }

  if (!is_initialized)
  {
    errors << wasp::lsp::m_error_prefix << "Server needs to be initialized" << std::endl;
    return false;
  }

  if (!is_document_open)
  {
    errors << wasp::lsp::m_error_prefix << "Server has no open document" << std::endl;
    return false;
  }

  bool pass = true;

  // TODO - hook up this capability by adding server specific logic here

  return pass;
}

bool
MooseServer::gatherDocumentSymbols(wasp::DataArray & documentSymbols)
{
  if (!_is_setup)
  {
    errors << wasp::lsp::m_error_prefix << "Server needs to be setup" << std::endl;
    return false;
  }

  if (!is_initialized)
  {
    errors << wasp::lsp::m_error_prefix << "Server needs to be initialized" << std::endl;
    return false;
  }

  if (!is_document_open)
  {
    errors << wasp::lsp::m_error_prefix << "Server has no open document" << std::endl;
    return false;
  }

  if (!_check_app || !_check_app->parser()._root)
  {
    return true;
  }

  bool pass = true;

  wasp::HITNodeView view_root = _check_app->parser()._root->getNodeView();

  for (const auto i : make_range(view_root.child_count()))
  {
    wasp::HITNodeView view_child = view_root.child_at(i);

    // get child name / detail / line / column / last_line / last_column

    std::string name = view_child.name();
    std::string detail = view_child.path();
    int line = view_child.line();
    int column = view_child.column();
    int last_line = view_child.last_line();
    int last_column = view_child.last_column();

    // according to the protocol line and column number are zero based

    line--;
    column--;
    last_line--;
    last_column--;

    // according to the protocol last_column is right AFTER last character

    last_column++;

    // build document symbol object from node child info and push to array

    documentSymbols.push_back(wasp::DataObject());

    wasp::DataObject * data_child = documentSymbols.back().to_object();

    pass &= wasp::lsp::buildDocumentSymbolObject(*data_child,
                                                 errors,
                                                 name,
                                                 detail,
                                                 1,
                                                 false,
                                                 line,
                                                 column,
                                                 last_line,
                                                 last_column,
                                                 line,
                                                 column,
                                                 last_line,
                                                 last_column);

    // call method to recursively fill document symbols for each node child

    pass &= traverseParseTreeAndFillSymbols(view_child, *data_child);
  }

  return pass;
}

bool
MooseServer::traverseParseTreeAndFillSymbols(wasp::HITNodeView view_parent,
                                             wasp::DataObject & data_parent)
{
  bool pass = true;

  for (const auto i : make_range(view_parent.child_count()))
  {
    wasp::HITNodeView view_child = view_parent.child_at(i);

    // get child name / detail / line / column / last_line / last_column

    std::string name = view_child.name();
    std::string detail = view_child.path();
    int line = view_child.line();
    int column = view_child.column();
    int last_line = view_child.last_line();
    int last_column = view_child.last_column();

    // according to the protocol line and column number are zero based

    line--;
    column--;
    last_line--;
    last_column--;

    // according to the protocol last_column is right AFTER last character

    last_column++;

    // build document symbol object from node child info and push to array

    wasp::DataObject & data_child = wasp::lsp::addDocumentSymbolChild(data_parent);

    pass &= wasp::lsp::buildDocumentSymbolObject(data_child,
                                                 errors,
                                                 name,
                                                 detail,
                                                 1,
                                                 false,
                                                 line,
                                                 column,
                                                 last_line,
                                                 last_column,
                                                 line,
                                                 column,
                                                 last_line,
                                                 last_column);

    // call method to recursively fill document symbols for each node child

    pass &= traverseParseTreeAndFillSymbols(view_child, data_child);
  }

  return pass;
}

#endif
