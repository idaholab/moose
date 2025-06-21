//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseServer.h"
#include "Moose.h"
#include "AppFactory.h"
#include "Syntax.h"
#include "ActionFactory.h"
#include "Factory.h"
#include "InputParameters.h"
#include "MooseUtils.h"
#include "MooseEnum.h"
#include "MultiMooseEnum.h"
#include "ExecFlagEnum.h"
#include "JsonSyntaxTree.h"
#include "FileLineInfo.h"
#include "CommandLine.h"
#include "Parser.h"
#include "pcrecpp.h"
#include "hit/hit.h"
#include "wasphit/HITInterpreter.h"
#include "waspcore/utils.h"
#include <algorithm>
#include <vector>
#include <sstream>
#include <iostream>
#include <functional>

MooseServer::MooseServer(MooseApp & moose_app)
  : _moose_app(moose_app),
    _connection(std::make_shared<wasp::lsp::IOStreamConnection>(this)),
    _formatting_tab_size(0)
{
  // set server capabilities to receive full input text when changed
  server_capabilities[wasp::lsp::m_text_doc_sync] = wasp::DataObject();
  server_capabilities[wasp::lsp::m_text_doc_sync][wasp::lsp::m_open_close] = true;
  server_capabilities[wasp::lsp::m_text_doc_sync][wasp::lsp::m_change] = wasp::lsp::m_change_full;

  // notify completion, symbol, formatting, definition capabilities support
  server_capabilities[wasp::lsp::m_completion_provider] = wasp::DataObject();
  server_capabilities[wasp::lsp::m_completion_provider][wasp::lsp::m_resolve_provider] = false;
  server_capabilities[wasp::lsp::m_doc_symbol_provider] = true;
  server_capabilities[wasp::lsp::m_doc_format_provider] = true;
  server_capabilities[wasp::lsp::m_definition_provider] = true;
  server_capabilities[wasp::lsp::m_references_provider] = true;
  server_capabilities[wasp::lsp::m_hover_provider] = true;
}

bool
MooseServer::parseDocumentForDiagnostics(wasp::DataArray & diagnosticsList)
{
  // Reset old parsers and applications if we have them
  if (const auto it = _check_state.find(document_path); it != _check_state.end())
    _check_state.erase(it);

  // strip prefix from document uri if it exists to get parse file path
  std::string parse_file_path = document_path;
  pcrecpp::RE("(.*://)(.*)").Replace("\\2", &parse_file_path);

  bool pass = true;

  // Adds a single diagnostic
  const auto diagnostic = [this, &diagnosticsList, &pass](const std::string & message,
                                                          const int start_line,
                                                          const int start_column,
                                                          const std::optional<int> end_line = {},
                                                          const std::optional<int> end_column = {})
  {
    diagnosticsList.push_back(wasp::DataObject());
    auto & diagnostic = *diagnosticsList.back().to_object();
    pass &= wasp::lsp::buildDiagnosticObject(diagnostic,
                                             errors,
                                             start_line,
                                             start_column,
                                             end_line ? *end_line : start_line,
                                             end_column ? *end_column : start_column,
                                             1,
                                             "moose_srv",
                                             "check_inp",
                                             message);
  };

  // Adds a diagnostic on line zero
  const auto zero_line_diagnostic = [&diagnostic](const std::string & message)
  { diagnostic(message, 0, 0); };

  // Adds a diagnostic from a hit node, if the context of the hit node is valid
  const auto hit_node_diagnostic = [&zero_line_diagnostic, &diagnostic, &parse_file_path](
                                       const hit::Node * const node, const std::string & message)
  {
    // No node, root node, wrong file, or no line information: line zero diagnostic
    if (!node || node->isRoot() || node->filename() != parse_file_path || !node->line() ||
        !node->column())
      zero_line_diagnostic(message);
    // Have file and line context, diagnostic there
    else
      diagnostic(message, node->line() - 1, node->column() - 1);
  };

  // Adds a diagnostic from a hit::ErrorMessage if the context is valid
  const auto hit_error_message_diagnostic =
      [&diagnostic, &zero_line_diagnostic, &parse_file_path](const hit::ErrorMessage & err)
  {
    // Has a filename
    if (err.filename)
    {
      // For the open file
      if (*err.filename == parse_file_path)
      {
        // Has line information that is valid
        if (err.lineinfo && err.lineinfo->start_line && err.lineinfo->start_column &&
            err.lineinfo->end_line && err.lineinfo->end_column)
        {
          diagnostic(err.message,
                     err.lineinfo->start_line - 1,
                     err.lineinfo->start_column - 1,
                     err.lineinfo->end_line - 1,
                     err.lineinfo->end_column - 1);
          return;
        }
      }
      // Has a file but not for this file, no diagnostic
      else
        return;
    }

    // Don't have a filename, or have a filename that is this file without line info
    zero_line_diagnostic(err.prefixed_message);
  };

  // Runs a try catch loop with the given action, collecting diagnostics
  // from the known exceptions; returns a bool that is true if we executed
  // without throwing anything
  const auto try_catch = [&hit_error_message_diagnostic,
                          &hit_node_diagnostic,
                          &zero_line_diagnostic](const auto & action) -> bool
  {
    const bool cached_throw_on_error = Moose::_throw_on_error;
    Moose::_throw_on_error = true;
    bool threw = true;

    try
    {
      action();
      threw = false;
    }
    // Will be thrown from the Parser while building the tree or
    // by the builder while building the input parameters
    catch (Parser::Error & err)
    {
      for (const auto & error_message : err.error_messages)
        hit_error_message_diagnostic(error_message);
    }
    // Will be thrown by mooseError() when _throw_on_error is set
    // to true, hopefully with hit node context
    catch (MooseRuntimeError & err)
    {
      hit_node_diagnostic(err.getNode(), err.what());
    }
    // General catch all for everything else without context
    catch (std::exception & err)
    {
      zero_line_diagnostic(err.what());
    }

    Moose::_throw_on_error = cached_throw_on_error;
    return !threw;
  };

  // Setup command line (needed by the Parser)
  auto command_line = std::make_unique<CommandLine>(_moose_app.commandLine()->getArguments());
  if (command_line->hasArgument("--language-server"))
    command_line->removeArgument("--language-server");
  command_line->addArgument("--check-input");
  command_line->addArgument("--error-unused");
  command_line->addArgument("--error");
  command_line->addArgument("--color=off");
  command_line->addArgument("--disable-perf-graph-live");
  command_line->parse();

  // Setup the parser that will be used in the app
  auto parser = std::make_shared<Parser>(parse_file_path, document_text);
  mooseAssert(parser->getInputFileNames()[0] == parse_file_path, "Should be consistent");
  parser->setCommandLineParams(command_line->buildHitParams());
  parser->setThrowOnError(true);

  // Try to parse the document
  const bool parse_success = try_catch([&parser]() { parser->parse(); });
  // If the Parser has a valid root, store it because we can use it
  // in the future (hover text etc with a partially complete document)
  CheckState * state = nullptr;
  if (auto parser_root_ptr = parser->queryRoot())
    if (!parser_root_ptr->getNodeView().is_null())
    {
      auto it_inserted_pair = _check_state.emplace(document_path, parser);
      mooseAssert(it_inserted_pair.second, "Should not already exist");
      state = &it_inserted_pair.first->second;
    }

  // Failed to parse, don't bother building the app. But... we might
  // have a root node at least!
  if (!parse_success)
    return pass;

  // Setup application options (including the Parser that succeeded)
  InputParameters app_params = _moose_app.parameters();
  app_params.set<std::shared_ptr<Parser>>("_parser") = parser;
  app_params.set<std::shared_ptr<CommandLine>>("_command_line") = std::move(command_line);

  // Try to instantiate the application
  std::unique_ptr<MooseApp> app = nullptr;
  const auto do_build_app = [this, &app_params, &app]()
  {
    app = AppFactory::instance().create(_moose_app.type(),
                                        AppFactory::main_app_name,
                                        app_params,
                                        _moose_app.getCommunicator()->get());
  };
  if (!try_catch(do_build_app))
  {
    if (app)
      app.reset();
    return pass;
  }

  // Store the app
  state->app = std::move(app);

  // Run the application, which will run the Builder
  const auto do_run_app = [this]() { getCheckApp().run(); };
  if (!try_catch(do_run_app))
    state->app.reset(); // destroy if we failed to build

  return pass;
}

bool
MooseServer::updateDocumentTextChanges(const std::string & replacement_text,
                                       int /* start_line */,
                                       int /* start_character */,
                                       int /* end_line */,
                                       int /* end_character*/,
                                       int /* range_length*/)
{
  // replacement text swaps full document as indicated in server capabilities
  document_text = replacement_text;

  return true;
}

bool
MooseServer::gatherDocumentCompletionItems(wasp::DataArray & completionItems,
                                           bool & is_incomplete,
                                           int line,
                                           int character)
{
  auto root_ptr = queryRoot();

  // add only root level blocks to completion list when parser root is null
  if (!root_ptr)
    return addSubblocksToList(completionItems, "/", line, character, line, character, "", false);
  auto & root = *root_ptr;

  // lambdas that will be used for checking completion request context type
  auto is_request_in_open_block = [](wasp::HITNodeView request_context) {
    return request_context.type() == wasp::OBJECT || request_context.type() == wasp::DOCUMENT_ROOT;
  };
  auto is_request_on_param_decl = [](wasp::HITNodeView request_context)
  {
    return request_context.type() == wasp::DECL && request_context.has_parent() &&
           (request_context.parent().type() == wasp::KEYED_VALUE ||
            request_context.parent().type() == wasp::ARRAY);
  };
  auto is_request_on_block_decl = [](wasp::HITNodeView request_context)
  {
    return request_context.type() == wasp::DECL && request_context.has_parent() &&
           request_context.parent().type() == wasp::OBJECT;
  };

  // get document tree root used to find node under request line and column
  wasp::HITNodeView view_root = root.getNodeView();
  wasp::HITNodeView request_context;

  // find node under request location if it is not past all defined content
  if (line + 1 < (int)view_root.last_line() ||
      (line + 1 == (int)view_root.last_line() && character <= (int)view_root.last_column()))
    request_context = wasp::findNodeUnderLineColumn(view_root, line + 1, character + 1);

  // otherwise find last node in document with last line and column of tree
  else
  {
    request_context =
        wasp::findNodeUnderLineColumn(view_root, view_root.last_line(), view_root.last_column());

    // change context to be parent block or grandparent if block terminator
    wasp::HITNodeView object_context = request_context;
    while (object_context.type() != wasp::OBJECT && object_context.has_parent())
      object_context = object_context.parent();
    if (request_context.type() == wasp::OBJECT_TERM && object_context.has_parent())
      object_context = object_context.parent();
    request_context = object_context;
  }

  // change context to equal sign if it is preceding node and in open block
  if (is_request_in_open_block(request_context))
  {
    wasp::HITNodeView backup_context = request_context;
    for (int backup_char = character; backup_context == request_context && --backup_char > 0;)
      backup_context = wasp::findNodeUnderLineColumn(request_context, line + 1, backup_char + 1);
    if (backup_context.type() == wasp::ASSIGN || backup_context.type() == wasp::OVERRIDE_ASSIGN)
      request_context = backup_context;
  }

  // use request context type to set up replacement range and prefix filter
  int replace_line_beg = line;
  int replace_char_beg = character;
  int replace_line_end = line;
  int replace_char_end = character;
  std::string filtering_prefix;
  if (request_context.type() == wasp::DECL || request_context.type() == wasp::VALUE)
  {
    // completion on existing block name, parameter name, or value replaces
    replace_line_beg = request_context.line() - 1;
    replace_char_beg = request_context.column() - 1;
    replace_line_end = request_context.last_line() - 1;
    replace_char_end = request_context.last_column();
    filtering_prefix = request_context.data();

    // empty block name columns are same as bracket so bump replace columns
    if (is_request_on_block_decl(request_context) && filtering_prefix.empty())
    {
      replace_char_beg++;
      replace_char_end++;
    }
  }

  // get name of request context direct parent node so it can be used later
  const auto & parent_name = request_context.has_parent() ? request_context.parent().name() : "";

  // get object context and value of type parameter for request if provided
  wasp::HITNodeView object_context = request_context;
  while (object_context.type() != wasp::OBJECT && object_context.has_parent())
    object_context = object_context.parent();
  if (is_request_on_block_decl(request_context))
    object_context = object_context.parent();
  const std::string & object_path = object_context.path();
  wasp::HITNodeView type_node = object_context.first_child_by_name("type");
  const std::string & object_type =
      type_node.is_null() ? "" : wasp::strip_quotes(hit::extractValue(type_node.data()));

  // get set of all parameter and subblock names already specified in input
  std::set<std::string> existing_params, existing_subblocks;
  getExistingInput(object_context, existing_params, existing_subblocks);

  // set used to gather all parameters valid from object context of request
  InputParameters valid_params = emptyInputParameters();

  // set used to gather MooseObjectAction tasks to verify object parameters
  std::set<std::string> obj_act_tasks;

  // get set of global parameters, action parameters, and object parameters
  getAllValidParameters(valid_params, object_path, object_type, obj_act_tasks);

  bool pass = true;

  // add gathered parameters to completion list with input range and prefix
  if (is_request_in_open_block(request_context) || is_request_on_param_decl(request_context))
    pass &= addParametersToList(completionItems,
                                valid_params,
                                existing_params,
                                replace_line_beg,
                                replace_char_beg,
                                replace_line_end,
                                replace_char_end,
                                filtering_prefix);

  // add all valid subblocks to completion list with input range and prefix
  if (is_request_in_open_block(request_context) || is_request_on_param_decl(request_context) ||
      is_request_on_block_decl(request_context))
    pass &= addSubblocksToList(completionItems,
                               object_path,
                               replace_line_beg,
                               replace_char_beg,
                               replace_line_end,
                               replace_char_end,
                               filtering_prefix,
                               is_request_on_block_decl(request_context));

  // add valid parameter value options to completion list using input range
  if ((request_context.type() == wasp::VALUE || request_context.type() == wasp::ASSIGN ||
       request_context.type() == wasp::OVERRIDE_ASSIGN) &&
      valid_params.getParametersList().count(parent_name))
    pass &= addValuesToList(completionItems,
                            valid_params,
                            existing_params,
                            existing_subblocks,
                            parent_name,
                            obj_act_tasks,
                            object_path,
                            replace_line_beg,
                            replace_char_beg,
                            replace_line_end,
                            replace_char_end);

  is_incomplete = !pass;

  return pass;
}

void
MooseServer::getExistingInput(wasp::HITNodeView parent_node,
                              std::set<std::string> & existing_params,
                              std::set<std::string> & existing_subblocks)
{
  // gather names of all parameters and subblocks provided in input context
  for (auto itr = parent_node.begin(); itr != parent_node.end(); itr.next())
  {
    auto child_node = itr.get();

    // add key value or array type as parameter and object type as subblock
    if (child_node.type() == wasp::KEYED_VALUE || child_node.type() == wasp::ARRAY)
      existing_params.insert(child_node.name());
    else if (child_node.type() == wasp::OBJECT)
      existing_subblocks.insert(child_node.name());
  }
}

void
MooseServer::getAllValidParameters(InputParameters & valid_params,
                                   const std::string & object_path,
                                   const std::string & object_type,
                                   std::set<std::string> & obj_act_tasks)
{
  // gather global parameters then action parameters then object parameters
  valid_params += Moose::Builder::validParams();
  getActionParameters(valid_params, object_path, obj_act_tasks);
  getObjectParameters(valid_params, object_type, obj_act_tasks);
}

void
MooseServer::getActionParameters(InputParameters & valid_params,
                                 const std::string & object_path,
                                 std::set<std::string> & obj_act_tasks)
{
  Syntax & syntax = _moose_app.syntax();
  ActionFactory & action_factory = _moose_app.getActionFactory();

  // get registered syntax path identifier using actual object context path
  bool is_parent;
  std::string registered_syntax = syntax.isAssociated(object_path, &is_parent);

  // use is_parent to skip action parameters when not explicitly registered
  if (!is_parent)
  {
    // get action objects associated with registered syntax path identifier
    auto action_range = syntax.getActions(registered_syntax);

    // traverse action objects for syntax to gather valid action parameters
    for (auto action_iter = action_range.first; action_iter != action_range.second; action_iter++)
    {
      const std::string & action_name = action_iter->second._action;

      // use action name to get set of valid parameters from action factory
      InputParameters action_params = action_factory.getValidParams(action_name);

      // gather all MooseObjectAction tasks for verifying object parameters
      if (action_params.have_parameter<bool>("isObjectAction"))
      {
        if (action_params.get<bool>("isObjectAction"))
        {
          std::set<std::string> tasks_by_actions = action_factory.getTasksByAction(action_name);
          obj_act_tasks.insert(tasks_by_actions.begin(), tasks_by_actions.end());
        }

        // filter parameter from completion list as it is not used in input
        action_params.remove("isObjectAction");
      }

      // add parameters from action to full valid collection being gathered
      valid_params += action_params;
    }
  }
}

void
MooseServer::getObjectParameters(InputParameters & valid_params,
                                 std::string object_type,
                                 const std::set<std::string> & obj_act_tasks)
{
  Syntax & syntax = _moose_app.syntax();
  Factory & factory = _moose_app.getFactory();

  // use type parameter default if it exists and is not provided from input
  if (object_type.empty() && valid_params.have_parameter<std::string>("type") &&
      !valid_params.get<std::string>("type").empty())
  {
    object_type = valid_params.get<std::string>("type");

    // make type parameter not required in input since it has default value
    valid_params.makeParamNotRequired("type");
  }

  // check if object type has been registered to prevent unregistered error
  if (factory.isRegistered(object_type))
  {
    // use object type to get set of valid parameters registered in factory
    InputParameters object_params = factory.getValidParams(object_type);

    // check if object has base associated with any MooseObjectAction tasks
    if (object_params.hasBase())
    {
      const std::string & moose_base = object_params.getBase();

      for (const auto & obj_act_task : obj_act_tasks)
      {
        if (syntax.verifyMooseObjectTask(moose_base, obj_act_task))
        {
          // add parameters from object to valid collection if base matches
          valid_params += object_params;
          break;
        }
      }
    }
  }

  // make parameters from list of those set by action not required in input
  if (valid_params.have_parameter<std::vector<std::string>>("_object_params_set_by_action"))
  {
    auto names = valid_params.get<std::vector<std::string>>("_object_params_set_by_action");
    for (const auto & name : names)
      valid_params.makeParamNotRequired(name);

    // filter parameter from completion list since it is not used for input
    valid_params.remove("_object_params_set_by_action");
  }
}

bool
MooseServer::addParametersToList(wasp::DataArray & completionItems,
                                 const InputParameters & valid_params,
                                 const std::set<std::string> & existing_params,
                                 int replace_line_beg,
                                 int replace_char_beg,
                                 int replace_line_end,
                                 int replace_char_end,
                                 const std::string & filtering_prefix)
{
  bool pass = true;

  // walk over collection of all valid parameters and build completion list
  for (const auto & valid_params_iter : valid_params)
  {
    const std::string & param_name = valid_params_iter.first;
    bool deprecated = valid_params.isParamDeprecated(param_name);
    bool is_private = valid_params.isPrivate(param_name);

    // filter out parameters that are deprecated, private, or already exist
    if (deprecated || is_private || existing_params.count(param_name))
      continue;

    // filter out parameters that do not begin with prefix if one was given
    if (param_name.rfind(filtering_prefix, 0) != 0)
      continue;

    // process parameter description and type to use in input default value
    std::string dirty_type = valid_params.type(param_name);
    std::string clean_type = MooseUtils::prettyCppType(dirty_type);
    std::string basic_type = JsonSyntaxTree::basicCppType(clean_type);
    std::string doc_string = valid_params.getDocString(param_name);
    MooseUtils::escape(doc_string);

    // use basic type to decide if parameter is array and quotes are needed
    bool is_array = basic_type.compare(0, 6, "Array:") == 0;

    // remove any array prefixes from basic type string and leave base type
    pcrecpp::RE("(Array:)*(.*)").GlobalReplace("\\2", &basic_type);

    // prepare clean cpp type string to be used for key to find input paths
    pcrecpp::RE(".+<([A-Za-z0-9_' ':]*)>.*").GlobalReplace("\\1", &clean_type);

    // decide completion item kind that client may use to display list icon
    int complete_kind = getCompletionItemKind(valid_params, param_name, clean_type, true);

    // default value for completion to be built using parameter information
    std::string default_value;

    // first if parameter default is set then use it to build default value
    if (valid_params.isParamValid(param_name))
    {
      default_value = JsonSyntaxTree::buildOutputString(valid_params_iter);
      default_value = MooseUtils::trim(default_value);
    }

    // otherwise if parameter has coupled default then use as default value
    else if (valid_params.hasDefaultCoupledValue(param_name))
    {
      std::ostringstream oss;
      oss << valid_params.defaultCoupledValue(param_name);
      default_value = oss.str();
    }

    // switch 1 to true or 0 to false if boolean parameter as default value
    if (basic_type == "Boolean" && default_value == "1")
      default_value = "true";
    else if (basic_type == "Boolean" && default_value == "0")
      default_value = "false";

    // wrap default value with single quotes if it exists and type is array
    std::string array_quote = is_array && !default_value.empty() ? "'" : "";

    // choose format of insertion text based on if client supports snippets
    int text_format;
    std::string insert_text;
    if (client_snippet_support && !default_value.empty())
    {
      text_format = wasp::lsp::m_text_format_snippet;
      insert_text = param_name + " = " + array_quote + "${1:" + default_value + "}" + array_quote;
    }
    else
    {
      text_format = wasp::lsp::m_text_format_plaintext;
      insert_text = param_name + " = " + array_quote + default_value + array_quote;
    }
    // finally build full insertion from parameter name, quote, and default

    // add parameter label, insert text, and description to completion list
    completionItems.push_back(wasp::DataObject());
    wasp::DataObject * item = completionItems.back().to_object();
    pass &= wasp::lsp::buildCompletionObject(*item,
                                             errors,
                                             param_name,
                                             replace_line_beg,
                                             replace_char_beg,
                                             replace_line_end,
                                             replace_char_end,
                                             insert_text,
                                             complete_kind,
                                             "",
                                             doc_string,
                                             false,
                                             false,
                                             text_format);
  }

  return pass;
}

bool
MooseServer::addSubblocksToList(wasp::DataArray & completionItems,
                                const std::string & object_path,
                                int replace_line_beg,
                                int replace_char_beg,
                                int replace_line_end,
                                int replace_char_end,
                                const std::string & filtering_prefix,
                                bool request_on_block_decl)
{
  Syntax & syntax = _moose_app.syntax();

  // set used to prevent reprocessing syntax paths for more than one action
  std::set<std::string> syntax_paths_processed;

  // build map of all syntax paths to names for subblocks and save to reuse
  if (_syntax_to_subblocks.empty())
  {
    for (const auto & syntax_path_iter : syntax.getAssociatedActions())
    {
      std::string syntax_path = "/" + syntax_path_iter.first;

      // skip current syntax path if already processed for different action
      if (!syntax_paths_processed.insert(syntax_path).second)
        continue;

      // walk backward through syntax path adding subblock names to parents
      for (std::size_t last_sep; (last_sep = syntax_path.find_last_of("/")) != std::string::npos;)
      {
        std::string subblock_name = syntax_path.substr(last_sep + 1);
        syntax_path = syntax_path.substr(0, last_sep);
        _syntax_to_subblocks[syntax_path].insert(subblock_name);
      }
    }
  }

  // get registered syntax from object path using map of paths to subblocks
  std::string registered_syntax = syntax.isAssociated(object_path, nullptr, _syntax_to_subblocks);

  bool pass = true;

  // walk over subblock names if found or at root and build completion list
  if (!registered_syntax.empty() || object_path == "/")
  {
    // choose format of insertion text based on if client supports snippets
    int text_format = client_snippet_support ? wasp::lsp::m_text_format_snippet
                                             : wasp::lsp::m_text_format_plaintext;

    for (const auto & subblock_name : _syntax_to_subblocks[registered_syntax])
    {
      // filter subblock if it does not begin with prefix and one was given
      if (subblock_name != "*" && subblock_name.rfind(filtering_prefix, 0) != 0)
        continue;

      std::string doc_string;
      std::string insert_text;
      int complete_kind;

      // build required parameter list for each block to use in insert text
      const std::string full_block_path = object_path + "/" + subblock_name;
      const std::string req_params = getRequiredParamsText(full_block_path, "", {}, "  ");

      // customize description and insert text for star and named subblocks
      if (subblock_name == "*")
      {
        doc_string = "custom user named block";
        insert_text = (request_on_block_decl ? "" : "[") +
                      (filtering_prefix.size() ? filtering_prefix : "block_name") + "]" +
                      req_params + "\n  " + (client_snippet_support ? "$0" : "") + "\n[]";
        complete_kind = wasp::lsp::m_comp_kind_variable;
      }
      else
      {
        doc_string = "application named block";
        insert_text = (request_on_block_decl ? "" : "[") + subblock_name + "]" + req_params +
                      "\n  " + (client_snippet_support ? "$0" : "") + "\n[]";
        complete_kind = wasp::lsp::m_comp_kind_struct;
      }

      // add subblock name, insert text, and description to completion list
      completionItems.push_back(wasp::DataObject());
      wasp::DataObject * item = completionItems.back().to_object();
      pass &= wasp::lsp::buildCompletionObject(*item,
                                               errors,
                                               subblock_name,
                                               replace_line_beg,
                                               replace_char_beg,
                                               replace_line_end,
                                               replace_char_end,
                                               insert_text,
                                               complete_kind,
                                               "",
                                               doc_string,
                                               false,
                                               false,
                                               text_format);
    }
  }

  return pass;
}

bool
MooseServer::addValuesToList(wasp::DataArray & completionItems,
                             const InputParameters & valid_params,
                             const std::set<std::string> & existing_params,
                             const std::set<std::string> & existing_subblocks,
                             const std::string & param_name,
                             const std::set<std::string> & obj_act_tasks,
                             const std::string & object_path,
                             int replace_line_beg,
                             int replace_char_beg,
                             int replace_line_end,
                             int replace_char_end)
{
  Syntax & syntax = _moose_app.syntax();
  Factory & factory = _moose_app.getFactory();

  // get clean type for path associations and basic type for boolean values
  std::string dirty_type = valid_params.type(param_name);
  std::string clean_type = MooseUtils::prettyCppType(dirty_type);
  std::string basic_type = JsonSyntaxTree::basicCppType(clean_type);

  // remove any array prefixes from basic type string and replace with base
  pcrecpp::RE("(Array:)*(.*)").GlobalReplace("\\2", &basic_type);

  // prepare clean cpp type string to be used for a key to find input paths
  pcrecpp::RE(".+<([A-Za-z0-9_' ':]*)>.*").GlobalReplace("\\1", &clean_type);

  // decide completion item kind that client may use to display a list icon
  int complete_kind = getCompletionItemKind(valid_params, param_name, clean_type, false);

  // map used to gather options and descriptions for value completion items
  std::map<std::string, std::string> options_and_descs;

  // first if parameter name is active or inactive then use input subblocks
  if (param_name == "active" || param_name == "inactive")
    for (const auto & subblock_name : existing_subblocks)
      options_and_descs[subblock_name] = "subblock name";

  // otherwise if parameter type is boolean then use true and false strings
  else if (basic_type == "Boolean")
  {
    options_and_descs["true"];
    options_and_descs["false"];
  }

  // otherwise if parameter type is one of the enums then use valid options
  else if (valid_params.have_parameter<MooseEnum>(param_name))
    getEnumsAndDocs(valid_params.get<MooseEnum>(param_name), options_and_descs);
  else if (valid_params.have_parameter<MultiMooseEnum>(param_name))
    getEnumsAndDocs(valid_params.get<MultiMooseEnum>(param_name), options_and_descs);
  else if (valid_params.have_parameter<ExecFlagEnum>(param_name))
    getEnumsAndDocs(valid_params.get<ExecFlagEnum>(param_name), options_and_descs);
  else if (valid_params.have_parameter<std::vector<MooseEnum>>(param_name))
    getEnumsAndDocs(valid_params.get<std::vector<MooseEnum>>(param_name)[0], options_and_descs);

  // otherwise if parameter name is type then use all verified object names
  else if (param_name == "type")
  {
    // walk over entire set of objects that have been registered in factory
    for (const auto & objects_iter : factory.registeredObjects())
    {
      const std::string & object_name = objects_iter.first;
      const InputParameters & object_params = objects_iter.second->buildParameters();

      // build required parameter list for each block to use in insert text
      std::string req_params = getRequiredParamsText(object_path, object_name, existing_params, "");
      req_params += req_params.size() ? "\n" + std::string(client_snippet_support ? "$0" : "") : "";

      // check if object has registered base parameter that can be verified
      if (!object_params.hasBase())
        continue;
      const std::string & moose_base = object_params.getBase();

      // walk over gathered MooseObjectAction tasks and add if base matches
      for (const auto & obj_act_task : obj_act_tasks)
      {
        if (!syntax.verifyMooseObjectTask(moose_base, obj_act_task))
          continue;
        std::string type_description = object_params.getClassDescription();
        MooseUtils::escape(type_description);
        options_and_descs[object_name + req_params] = type_description;
        break;
      }
    }
  }

  // otherwise if parameter type has any associated syntax then use lookups
  else
  {
    // build map of parameter types to input lookup paths and save to reuse
    if (_type_to_input_paths.empty())
    {
      for (const auto & associated_types_iter : syntax.getAssociatedTypes())
      {
        const std::string & type = associated_types_iter.second;
        const std::string & path = associated_types_iter.first;
        _type_to_input_paths[type].insert(path);
      }
    }

    // check for input lookup paths that are associated with parameter type
    const auto & input_path_iter = _type_to_input_paths.find(clean_type);

    if (input_path_iter != _type_to_input_paths.end())
    {
      wasp::HITNodeView view_root = getRoot().getNodeView();

      // walk over all syntax paths that are associated with parameter type
      for (const auto & input_path : input_path_iter->second)
      {
        // use wasp siren to gather all input values at current lookup path
        wasp::SIRENInterpreter<> selector;
        if (!selector.parseString(input_path))
          continue;
        wasp::SIRENResultSet<wasp::HITNodeView> results;
        std::size_t count = selector.evaluate(view_root, results);

        // walk over results and add each input value found at current path
        for (std::size_t i = 0; i < count; i++)
          if (results.adapted(i).type() == wasp::OBJECT)
            options_and_descs[results.adapted(i).name()] = "from /" + input_path;
      }
    }
  }

  // choose format of insertion text based on if client has snippet support
  int text_format = client_snippet_support ? wasp::lsp::m_text_format_snippet
                                           : wasp::lsp::m_text_format_plaintext;

  bool pass = true;

  // walk over pairs of options with descriptions and build completion list
  for (const auto & option_and_desc : options_and_descs)
  {
    const std::string & insert_text = option_and_desc.first;
    const std::string & option_name = insert_text.substr(0, insert_text.find('\n'));
    const std::string & description = option_and_desc.second;

    // add option name, insertion range, and description to completion list
    completionItems.push_back(wasp::DataObject());
    wasp::DataObject * item = completionItems.back().to_object();
    pass &= wasp::lsp::buildCompletionObject(*item,
                                             errors,
                                             option_name,
                                             replace_line_beg,
                                             replace_char_beg,
                                             replace_line_end,
                                             replace_char_end,
                                             insert_text,
                                             complete_kind,
                                             "",
                                             description,
                                             false,
                                             false,
                                             text_format);
  }

  return pass;
}

template <typename MooseEnumType>
void
MooseServer::getEnumsAndDocs(MooseEnumType & moose_enum_param,
                             std::map<std::string, std::string> & options_and_descs)
{
  // get map that contains any documentation strings provided for each item
  const auto & enum_docs = moose_enum_param.getItemDocumentation();

  // walk over enums filling map with options and any provided descriptions
  for (const auto & item : moose_enum_param.items())
    options_and_descs[item.name()] = enum_docs.count(item) ? enum_docs.at(item) : "";
}

bool
MooseServer::gatherDocumentDefinitionLocations(wasp::DataArray & definitionLocations,
                                               int line,
                                               int character)
{
  Factory & factory = _moose_app.getFactory();

  // return without any definition locations added when parser root is null
  auto root_ptr = queryRoot();
  if (!root_ptr)
    return true;
  auto & root = *root_ptr;

  // find hit node for zero based request line and column number from input
  wasp::HITNodeView view_root = root.getNodeView();
  wasp::HITNodeView request_context =
      wasp::findNodeUnderLineColumn(view_root, line + 1, character + 1);

  // return without any definition locations added when node not value type
  if (request_context.type() != wasp::VALUE)
    return true;

  // get name of parameter node parent of value and value string from input
  std::string param_name = request_context.has_parent() ? request_context.parent().name() : "";
  std::string val_string = request_context.last_as_string();

  // add source code location if type parameter with registered object name
  if (param_name == "type" && factory.isRegistered(val_string))
  {
    // get file path and line number of source code registering object type
    FileLineInfo file_line_info = factory.getLineInfo(val_string);

    // return without any definition locations added if file cannot be read
    if (!file_line_info.isValid() ||
        !MooseUtils::checkFileReadable(file_line_info.file(), false, false, false))
      return true;

    // add file scheme prefix to front of file path to build definition uri
    auto location_uri = wasp::lsp::m_uri_prefix + file_line_info.file();

    // add file uri and zero based line and column range to definition list
    definitionLocations.push_back(wasp::DataObject());
    wasp::DataObject * location = definitionLocations.back().to_object();
    return wasp::lsp::buildLocationObject(*location,
                                          errors,
                                          location_uri,
                                          file_line_info.line() - 1,
                                          0,
                                          file_line_info.line() - 1,
                                          1000);
  }

  // get object context and value of type parameter for request if provided
  wasp::HITNodeView object_context = request_context;
  while (object_context.type() != wasp::OBJECT && object_context.has_parent())
    object_context = object_context.parent();
  const std::string & object_path = object_context.path();
  wasp::HITNodeView type_node = object_context.first_child_by_name("type");
  const std::string & object_type =
      type_node.is_null() ? "" : wasp::strip_quotes(hit::extractValue(type_node.data()));

  // set used to gather all parameters valid from object context of request
  InputParameters valid_params = emptyInputParameters();

  // set used to gather MooseObjectAction tasks to verify object parameters
  std::set<std::string> obj_act_tasks;

  // get set of global parameters, action parameters, and object parameters
  getAllValidParameters(valid_params, object_path, object_type, obj_act_tasks);

  // set used to gather nodes from input lookups custom sorted by locations
  SortedLocationNodes location_nodes(
      [](const wasp::HITNodeView & l, const wasp::HITNodeView & r)
      {
        const std::string & l_file = l.node_pool()->stream_name();
        const std::string & r_file = r.node_pool()->stream_name();
        return (l_file < r_file || (l_file == r_file && l.line() < r.line()) ||
                (l_file == r_file && l.line() == r.line() && l.column() < r.column()));
      });

  // gather all lookup path nodes matching value if parameter name is valid
  for (const auto & valid_params_iter : valid_params)
  {
    if (valid_params_iter.first == param_name)
    {
      // get cpp type and prepare string for use as key finding input paths
      std::string dirty_type = valid_params.type(param_name);
      std::string clean_type = MooseUtils::prettyCppType(dirty_type);
      pcrecpp::RE(".+<([A-Za-z0-9_' ':]*)>.*").GlobalReplace("\\1", &clean_type);

      // get set of nodes from associated path lookups matching input value
      getInputLookupDefinitionNodes(location_nodes, clean_type, val_string);
      break;
    }
  }

  // add parameter declarator to set if none were gathered by input lookups
  if (location_nodes.empty() && request_context.has_parent() &&
      request_context.parent().child_count_by_name("decl"))
    location_nodes.insert(request_context.parent().first_child_by_name("decl"));

  // add locations to definition list using lookups or parameter declarator
  return addLocationNodesToList(definitionLocations, location_nodes);
}

void
MooseServer::getInputLookupDefinitionNodes(SortedLocationNodes & location_nodes,
                                           const std::string & clean_type,
                                           const std::string & val_string)
{
  Syntax & syntax = _moose_app.syntax();

  // build map from parameter types to input lookup paths and save to reuse
  if (_type_to_input_paths.empty())
  {
    for (const auto & associated_types_iter : syntax.getAssociatedTypes())
    {
      const std::string & type = associated_types_iter.second;
      const std::string & path = associated_types_iter.first;
      _type_to_input_paths[type].insert(path);
    }
  }

  // find set of input lookup paths that are associated with parameter type
  const auto & input_path_iter = _type_to_input_paths.find(clean_type);

  // return without any definition locations added when no paths associated
  if (input_path_iter == _type_to_input_paths.end())
    return;

  // get root node from input to use in input lookups with associated paths
  wasp::HITNodeView view_root = getRoot().getNodeView();

  // walk over all syntax paths that are associated with parameter type
  for (const auto & input_path : input_path_iter->second)
  {
    // use wasp siren to gather all nodes from current lookup path in input
    wasp::SIRENInterpreter<> selector;
    if (!selector.parseString(input_path))
      continue;
    wasp::SIRENResultSet<wasp::HITNodeView> results;
    std::size_t count = selector.evaluate(view_root, results);

    // walk over results and add nodes that have name matching value to set
    for (std::size_t i = 0; i < count; i++)
      if (results.adapted(i).type() == wasp::OBJECT && results.adapted(i).name() == val_string &&
          results.adapted(i).child_count_by_name("decl"))
        location_nodes.insert(results.adapted(i).first_child_by_name("decl"));
  }
}

bool
MooseServer::addLocationNodesToList(wasp::DataArray & defsOrRefsLocations,
                                    const SortedLocationNodes & location_nodes)
{
  bool pass = true;

  // walk over set of sorted nodes provided to add and build locations list
  for (const auto & location_nodes_iter : location_nodes)
  {
    // add file scheme prefix onto front of file path to build location uri
    auto location_uri = wasp::lsp::m_uri_prefix + location_nodes_iter.node_pool()->stream_name();

    // add file uri with zero based line and column range to locations list
    defsOrRefsLocations.push_back(wasp::DataObject());
    wasp::DataObject * location = defsOrRefsLocations.back().to_object();
    pass &= wasp::lsp::buildLocationObject(*location,
                                           errors,
                                           location_uri,
                                           location_nodes_iter.line() - 1,
                                           location_nodes_iter.column() - 1,
                                           location_nodes_iter.last_line() - 1,
                                           location_nodes_iter.last_column());
  }

  return pass;
}

bool
MooseServer::getHoverDisplayText(std::string & display_text, int line, int character)
{
  Factory & factory = _moose_app.getFactory();
  Syntax & syntax = _moose_app.syntax();

  // return and leave display text as empty string when parser root is null
  auto root_ptr = queryRoot();
  if (!root_ptr)
    return true;
  auto & root = *root_ptr;

  // find hit node for zero based request line and column number from input
  wasp::HITNodeView view_root = root.getNodeView();
  wasp::HITNodeView request_context =
      wasp::findNodeUnderLineColumn(view_root, line + 1, character + 1);

  // return and leave display text as empty string when not on key or value
  if ((request_context.type() != wasp::DECL && request_context.type() != wasp::VALUE) ||
      !request_context.has_parent() ||
      (request_context.parent().type() != wasp::KEYED_VALUE &&
       request_context.parent().type() != wasp::ARRAY))
    return true;

  // get name of parameter node and value string that is specified in input
  std::string paramkey = request_context.parent().name();
  std::string paramval = request_context.last_as_string();

  // get object context path and object type value for request if it exists
  wasp::HITNodeView object_context = request_context;
  while (object_context.type() != wasp::OBJECT && object_context.has_parent())
    object_context = object_context.parent();
  const std::string object_path = object_context.path();
  wasp::HITNodeView type_node = object_context.first_child_by_name("type");
  const std::string object_type =
      type_node.is_null() ? "" : wasp::strip_quotes(hit::extractValue(type_node.data()));

  // gather global, action, and object parameters in request object context
  InputParameters valid_params = emptyInputParameters();
  std::set<std::string> obj_act_tasks;
  getAllValidParameters(valid_params, object_path, object_type, obj_act_tasks);

  // use class description as display text when request is valid type value
  if (request_context.type() == wasp::VALUE && paramkey == "type" && factory.isRegistered(paramval))
  {
    const InputParameters & object_params = factory.getValidParams(paramval);
    if (object_params.hasBase())
    {
      const std::string & moose_base = object_params.getBase();
      for (const auto & obj_act_task : obj_act_tasks)
      {
        if (syntax.verifyMooseObjectTask(moose_base, obj_act_task))
        {
          display_text = object_params.getClassDescription();
          break;
        }
      }
    }
  }

  // use item documentation as display text when request is enum type value
  else if (request_context.type() == wasp::VALUE)
  {
    std::map<std::string, std::string> options_and_descs;
    if (valid_params.have_parameter<MooseEnum>(paramkey))
      getEnumsAndDocs(valid_params.get<MooseEnum>(paramkey), options_and_descs);
    else if (valid_params.have_parameter<MultiMooseEnum>(paramkey))
      getEnumsAndDocs(valid_params.get<MultiMooseEnum>(paramkey), options_and_descs);
    else if (valid_params.have_parameter<ExecFlagEnum>(paramkey))
      getEnumsAndDocs(valid_params.get<ExecFlagEnum>(paramkey), options_and_descs);
    else if (valid_params.have_parameter<std::vector<MooseEnum>>(paramkey))
      getEnumsAndDocs(valid_params.get<std::vector<MooseEnum>>(paramkey)[0], options_and_descs);
    if (options_and_descs.count(paramval))
      display_text = options_and_descs.find(paramval)->second;
  }

  // use parameter documentation as display text when request is valid name
  else if (request_context.type() == wasp::DECL && valid_params.getParametersList().count(paramkey))
    display_text = valid_params.getDocString(paramkey);

  MooseUtils::escape(display_text);
  return true;
}

bool
MooseServer::gatherDocumentReferencesLocations(wasp::DataArray & referencesLocations,
                                               int line,
                                               int character,
                                               bool include_declaration)
{
  Syntax & syntax = _moose_app.syntax();

  // return without adding any reference locations when parser root is null
  auto root_ptr = queryRoot();
  if (!root_ptr)
    return true;
  auto & root = *root_ptr;

  // find hit node for zero based request line and column number from input
  wasp::HITNodeView view_root = root.getNodeView();
  wasp::HITNodeView request_context =
      wasp::findNodeUnderLineColumn(view_root, line + 1, character + 1);

  // return without adding any references when request not block declarator
  if ((request_context.type() != wasp::DECL && request_context.type() != wasp::DOT_SLASH &&
       request_context.type() != wasp::LBRACKET && request_context.type() != wasp::RBRACKET) ||
      !request_context.has_parent() || request_context.parent().type() != wasp::OBJECT)
    return true;

  // get input path and block name of declarator located at request context
  const std::string & block_path = request_context.parent().path();
  const std::string & block_name = request_context.parent().name();

  // build map from input lookup paths to parameter types and save to reuse
  if (_input_path_to_types.empty())
    for (const auto & associated_types_iter : syntax.getAssociatedTypes())
    {
      const std::string & path = associated_types_iter.first;
      const std::string & type = associated_types_iter.second;
      _input_path_to_types[path].insert(type);
    }

  // get registered syntax from block path with map of input paths to types
  bool is_parent;
  std::string registered_syntax = syntax.isAssociated(block_path, &is_parent, _input_path_to_types);

  // return without adding any references if syntax has no types associated
  if (is_parent || !_input_path_to_types.count(registered_syntax))
    return true;

  // get set of parameter types which are associated with registered syntax
  const std::set<std::string> & target_types = _input_path_to_types.at(registered_syntax);

  // set used to gather nodes collected by value custom sorted by locations
  SortedLocationNodes match_nodes(
      [](const wasp::HITNodeView & l, const wasp::HITNodeView & r)
      {
        const std::string & l_file = l.node_pool()->stream_name();
        const std::string & r_file = r.node_pool()->stream_name();
        return (l_file < r_file || (l_file == r_file && l.line() < r.line()) ||
                (l_file == r_file && l.line() == r.line() && l.column() < r.column()));
      });

  // walk input recursively and gather all nodes that match value and types
  getNodesByValueAndTypes(match_nodes, view_root, block_name, target_types);

  // return without adding any references if no nodes match value and types
  if (match_nodes.empty())
    return true;

  // add request context node to set if declaration inclusion was specified
  if (include_declaration && request_context.parent().child_count_by_name("decl"))
    match_nodes.insert(request_context.parent().first_child_by_name("decl"));

  // add locations to references list with nodes that match value and types
  return addLocationNodesToList(referencesLocations, match_nodes);
}

void
MooseServer::getNodesByValueAndTypes(SortedLocationNodes & match_nodes,
                                     wasp::HITNodeView view_parent,
                                     const std::string & target_value,
                                     const std::set<std::string> & target_types)
{
  // walk over children of context to gather nodes matching value and types
  for (const auto & view_child : view_parent)
  {
    // check for parameter type match if node is value matching target data
    if (view_child.type() == wasp::VALUE && view_child.to_string() == target_value)
    {
      // get object context path and object type value of node if it exists
      wasp::HITNodeView object_context = view_child;
      while (object_context.type() != wasp::OBJECT && object_context.has_parent())
        object_context = object_context.parent();
      const std::string object_path = object_context.path();
      wasp::HITNodeView type_node = object_context.first_child_by_name("type");
      const std::string object_type =
          type_node.is_null() ? "" : wasp::strip_quotes(hit::extractValue(type_node.data()));

      // gather global, action, and object parameters for context of object
      InputParameters valid_params = emptyInputParameters();
      std::set<std::string> obj_act_tasks;
      getAllValidParameters(valid_params, object_path, object_type, obj_act_tasks);

      // get name from parent of current value node which is parameter node
      std::string param_name = view_child.has_parent() ? view_child.parent().name() : "";

      // get type of parameter and prepare string to check target set match
      std::string dirty_type = valid_params.type(param_name);
      std::string clean_type = MooseUtils::prettyCppType(dirty_type);
      pcrecpp::RE(".+<([A-Za-z0-9_' ':]*)>.*").GlobalReplace("\\1", &clean_type);

      // add input node to collection if its type is also in set of targets
      if (target_types.count(clean_type))
        match_nodes.insert(view_child);
    }

    // recurse deeper into input to search for matches if node has children
    if (!view_child.is_leaf())
      getNodesByValueAndTypes(match_nodes, view_child, target_value, target_types);
  }
}

bool
MooseServer::gatherDocumentFormattingTextEdits(wasp::DataArray & formattingTextEdits,
                                               int tab_size,
                                               bool /* insert_spaces */)
{
  // strip scheme prefix from document uri if it exists for parse file path
  std::string parse_file_path = document_path;
  pcrecpp::RE("(.*://)(.*)").Replace("\\2", &parse_file_path);

  // input check expanded any brace expressions in cached tree so reprocess
  std::stringstream input_errors, input_stream(getDocumentText());
  wasp::DefaultHITInterpreter interpreter(input_errors);

  // return without adding any formatting text edits if input parsing fails
  if (!interpreter.parseStream(input_stream, parse_file_path))
    return true;

  // return without adding any formatting text edits if parser root is null
  if (interpreter.root().is_null())
    return true;

  // get input root node line and column range to represent entire document
  wasp::HITNodeView view_root = interpreter.root();
  int document_start_line = view_root.line() - 1;
  int document_start_char = view_root.column() - 1;
  int document_last_line = view_root.last_line() - 1;
  int document_last_char = view_root.last_column();

  // set number of spaces for indentation and build formatted document text
  _formatting_tab_size = tab_size;
  std::size_t starting_line = view_root.line() - 1;
  std::string document_format = formatDocument(view_root, starting_line, 0);

  // add formatted text with whole line and column range to formatting list
  formattingTextEdits.push_back(wasp::DataObject());
  wasp::DataObject * item = formattingTextEdits.back().to_object();
  bool pass = wasp::lsp::buildTextEditObject(*item,
                                             errors,
                                             document_start_line,
                                             document_start_char,
                                             document_last_line,
                                             document_last_char,
                                             document_format);
  return pass;
}

std::string
MooseServer::formatDocument(wasp::HITNodeView parent, std::size_t & prev_line, std::size_t level)
{
  // build string of newline and indentation spaces from level and tab size
  std::string newline_indent = "\n" + std::string(level * _formatting_tab_size, ' ');

  // lambda to format include data by replacing consecutive spaces with one
  auto collapse_spaces = [](std::string string_copy)
  {
    pcrecpp::RE("\\s+").Replace(" ", &string_copy);
    return string_copy;
  };

  // formatted string that will be built recursively by appending each call
  std::string format_string;

  // walk over all children of this node context and build formatted string
  for (const auto i : make_range(parent.child_count()))
  {
    // walk must be index based to catch file include and skip its children
    wasp::HITNodeView child = parent.child_at(i);

    // add blank line if necessary after previous line and before this line
    std::string blank = child.line() > prev_line + 1 ? "\n" : "";

    // format include directive with indentation and collapse extra spacing
    if (child.type() == wasp::FILE)
      format_string += blank + newline_indent + MooseUtils::trim(collapse_spaces(child.data()));

    // format normal comment with indentation and inline comment with space
    else if (child.type() == wasp::COMMENT)
      format_string += (child.line() == prev_line ? " " : blank + newline_indent) +
                       MooseUtils::trim(child.data());

    // format object recursively with indentation and without legacy syntax
    else if (child.type() == wasp::OBJECT)
      format_string += blank + newline_indent + "[" + child.name() + "]" +
                       formatDocument(child, prev_line, level + 1) + newline_indent + "[]";

    // format keyed value with indentation and calling reusable hit methods
    else if (child.type() == wasp::KEYED_VALUE || child.type() == wasp::ARRAY)
    {
      const std::string prefix = newline_indent + child.name() + " = ";

      const std::string render_val = hit::extractValue(child.data());
      std::size_t val_column = child.child_count() > 2 ? child.child_at(2).column() : 0;
      std::size_t prefix_len = prefix.size() - 1;

      format_string += blank + prefix + hit::formatValue(render_val, val_column, prefix_len);
    }

    // set previous line reference used for blank lines and inline comments
    prev_line = child.last_line();
  }

  // remove leading newline if this is level zero returning entire document
  return level != 0 ? format_string : format_string.substr(1);
}

bool
MooseServer::gatherDocumentSymbols(wasp::DataArray & documentSymbols)
{
  // return prior to starting document symbol tree when parser root is null
  auto root_ptr = queryRoot();
  if (!root_ptr)
    return true;
  auto & root = *root_ptr;

  wasp::HITNodeView view_root = root.getNodeView();

  bool pass = true;

  // walk over all children of root node context and build document symbols
  for (const auto i : make_range(view_root.child_count()))
  {
    // walk must be index based to catch file include and skip its children
    wasp::HITNodeView view_child = view_root.child_at(i);

    // set up name, zero based line and column range, kind, and detail info
    std::string name = view_child.name();
    int line = view_child.line() - 1;
    int column = view_child.column() - 1;
    int last_line = view_child.last_line() - 1;
    int last_column = view_child.last_column();
    int symbol_kind = getDocumentSymbolKind(view_child);
    std::string detail =
        !view_child.first_child_by_name("type").is_null()
            ? wasp::strip_quotes(hit::extractValue(view_child.first_child_by_name("type").data()))
            : "";

    // build document symbol object from node child info and push to array
    documentSymbols.push_back(wasp::DataObject());
    wasp::DataObject * data_child = documentSymbols.back().to_object();
    pass &= wasp::lsp::buildDocumentSymbolObject(*data_child,
                                                 errors,
                                                 (name.empty() ? "void" : name),
                                                 detail,
                                                 symbol_kind,
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
  // return without adding any children if parent node is file include type
  if (wasp::is_nested_file(view_parent))
    return true;

  bool pass = true;

  // walk over all children of this node context and build document symbols
  for (const auto i : make_range(view_parent.child_count()))
  {
    // walk must be index based to catch file include and skip its children
    wasp::HITNodeView view_child = view_parent.child_at(i);

    // set up name, zero based line and column range, kind, and detail info
    std::string name = view_child.name();
    int line = view_child.line() - 1;
    int column = view_child.column() - 1;
    int last_line = view_child.last_line() - 1;
    int last_column = view_child.last_column();
    int symbol_kind = getDocumentSymbolKind(view_child);
    std::string detail =
        !view_child.first_child_by_name("type").is_null()
            ? wasp::strip_quotes(hit::extractValue(view_child.first_child_by_name("type").data()))
            : "";

    // build document symbol object from node child info and push to array
    wasp::DataObject & data_child = wasp::lsp::addDocumentSymbolChild(data_parent);
    pass &= wasp::lsp::buildDocumentSymbolObject(data_child,
                                                 errors,
                                                 (name.empty() ? "void" : name),
                                                 detail,
                                                 symbol_kind,
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

int
MooseServer::getCompletionItemKind(const InputParameters & valid_params,
                                   const std::string & param_name,
                                   const std::string & clean_type,
                                   bool is_param)
{
  // set up completion item kind value that client may use for icon in list
  auto associated_types = _moose_app.syntax().getAssociatedTypes();
  if (is_param && valid_params.isParamRequired(param_name) &&
      !valid_params.isParamValid(param_name))
    return wasp::lsp::m_comp_kind_event;
  else if (param_name == "active" || param_name == "inactive")
    return wasp::lsp::m_comp_kind_class;
  else if (clean_type == "bool")
    return wasp::lsp::m_comp_kind_interface;
  else if (valid_params.have_parameter<MooseEnum>(param_name) ||
           valid_params.have_parameter<MultiMooseEnum>(param_name) ||
           valid_params.have_parameter<ExecFlagEnum>(param_name) ||
           valid_params.have_parameter<std::vector<MooseEnum>>(param_name))
    return is_param ? wasp::lsp::m_comp_kind_enum : wasp::lsp::m_comp_kind_enum_member;
  else if (param_name == "type")
    return wasp::lsp::m_comp_kind_type_param;
  else if (std::find_if(associated_types.begin(),
                        associated_types.end(),
                        [&](const auto & entry)
                        { return entry.second == clean_type; }) != associated_types.end())
    return wasp::lsp::m_comp_kind_reference;
  else
    return is_param ? wasp::lsp::m_comp_kind_keyword : wasp::lsp::m_comp_kind_value;
}

int
MooseServer::getDocumentSymbolKind(wasp::HITNodeView symbol_node)
{
  // lambdas that check if parameter is a boolean or number for symbol kind
  auto is_boolean = [](wasp::HITNodeView symbol_node)
  {
    bool convert;
    std::istringstream iss(MooseUtils::toLower(symbol_node.last_as_string()));
    return (iss >> std::boolalpha >> convert && !iss.fail());
  };
  auto is_number = [](wasp::HITNodeView symbol_node)
  {
    double convert;
    std::istringstream iss(symbol_node.last_as_string());
    return (iss >> convert && iss.eof());
  };

  // set up document symbol kind value that client may use for outline icon
  if (symbol_node.type() == wasp::OBJECT)
    return wasp::lsp::m_symbol_kind_struct;
  else if (symbol_node.type() == wasp::FILE)
    return wasp::lsp::m_symbol_kind_file;
  else if (symbol_node.type() == wasp::ARRAY)
    return wasp::lsp::m_symbol_kind_array;
  else if (symbol_node.type() == wasp::KEYED_VALUE && symbol_node.name() == std::string("type"))
    return wasp::lsp::m_symbol_kind_type_param;
  else if (symbol_node.type() == wasp::KEYED_VALUE && is_boolean(symbol_node))
    return wasp::lsp::m_symbol_kind_boolean;
  else if (symbol_node.type() == wasp::KEYED_VALUE && is_number(symbol_node))
    return wasp::lsp::m_symbol_kind_number;
  else if (symbol_node.type() == wasp::KEYED_VALUE)
    return wasp::lsp::m_symbol_kind_key;
  else if (symbol_node.type() == wasp::VALUE)
    return wasp::lsp::m_symbol_kind_string;
  else
    return wasp::lsp::m_symbol_kind_property;
}

std::string
MooseServer::getRequiredParamsText(const std::string & subblock_path,
                                   const std::string & subblock_type,
                                   const std::set<std::string> & existing_params,
                                   const std::string & indent_spaces)
{
  // gather global, action, and object parameters in request object context
  InputParameters valid_params = emptyInputParameters();
  std::set<std::string> obj_act_tasks;
  getAllValidParameters(valid_params, subblock_path, subblock_type, obj_act_tasks);

  // walk over collection of all parameters and build text of ones required
  std::string required_param_text;
  std::size_t param_index = 1;
  for (const auto & valid_params_iter : valid_params)
  {
    // skip parameter if deprecated, private, defaulted, optional, existing
    const std::string & param_name = valid_params_iter.first;
    if (!valid_params.isParamDeprecated(param_name) && !valid_params.isPrivate(param_name) &&
        !valid_params.isParamValid(param_name) && valid_params.isParamRequired(param_name) &&
        !existing_params.count(param_name))
    {
      std::string tab_stop = client_snippet_support ? "$" + std::to_string(param_index++) : "";
      required_param_text += "\n" + indent_spaces + param_name + " = " + tab_stop;
    }
  }

  return required_param_text;
}

// TODO RUN PARSER PARSE FIRST !!

hit::Node *
MooseServer::queryRoot()
{
  if (auto parser_ptr = queryCheckParser())
  {
#ifndef NDEBUG
    if (const auto app_ptr = queryCheckApp())
      mooseAssert(&app_ptr->parser() == parser_ptr, "App should have this parser");
#endif
    if (auto root_ptr = parser_ptr->queryRoot())
      if (!root_ptr->getNodeView().is_null())
        return root_ptr;
  }
  return nullptr;
}

MooseServer::CheckState *
MooseServer::queryCheckState()
{
  const auto it = _check_state.find(document_path);
  return it == _check_state.end() ? nullptr : &it->second;
}

MooseApp *
MooseServer::queryCheckApp()
{
  if (auto state = queryCheckState())
    return state->app.get();
  return nullptr;
}

Parser *
MooseServer::queryCheckParser()
{
  if (auto state = queryCheckState())
    return state->parser.get();
  return nullptr;
}

const std::string *
MooseServer::queryDocumentText()
{
  if (const auto parser = queryCheckParser())
  {
    const auto & text_vector = parser->getInputText();
    mooseAssert(text_vector.size() == 1, "Unexpected size");
    return &text_vector[0];
  }
  return nullptr;
}

MooseApp &
MooseServer::getCheckApp()
{
  if (auto app_ptr = queryCheckApp())
  {
    auto & app = *app_ptr;
    mooseAssert(queryCheckParser(), "Should have a parser");
    mooseAssert(&app.parser() == queryCheckParser(), "Parser should be the app's parser");
    return app;
  }
  mooseError("MooseServer::getCheckApp(): App not available");
}

Parser &
MooseServer::getCheckParser()
{
  if (auto parser_ptr = queryCheckParser())
  {
    auto & parser = *parser_ptr;
#ifndef NDEBUG
    if (const auto app_ptr = queryCheckApp())
      mooseAssert(&app_ptr->parser() == parser_ptr, "App should have this parser");
#endif
    return parser;
  }
  mooseError("MooseServer::getCheckParser(): Parser not available");
}

hit::Node &
MooseServer::getRoot()
{
  if (auto root_ptr = queryRoot())
    return *root_ptr;
  mooseError("MooseServer::getRoot(): Root not available");
}

const std::string &
MooseServer::getDocumentText()
{
  if (auto text_ptr = queryDocumentText())
    return *text_ptr;
  mooseError("MooseServer::getDocumentText(): Document text not available");
}
