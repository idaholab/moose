//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
#include "pcrecpp.h"
#include "waspcore/utils.h"
#include <algorithm>
#include <vector>
#include <sstream>

MooseServer::MooseServer(MooseApp & moose_app)
  : _moose_app(moose_app), _connection(std::make_shared<wasp::lsp::IOStreamConnection>(this))
{
  // set server capabilities to receive full input text when changed
  server_capabilities[wasp::lsp::m_text_doc_sync] = wasp::DataObject();
  server_capabilities[wasp::lsp::m_text_doc_sync][wasp::lsp::m_open_close] = true;
  server_capabilities[wasp::lsp::m_text_doc_sync][wasp::lsp::m_change] = wasp::lsp::m_change_full;
}

bool
MooseServer::parseDocumentForDiagnostics(wasp::DataArray & diagnosticsList)
{
  // strip prefix from document uri if it exists to get parse file path
  std::string parse_file_path = document_path;
  if (parse_file_path.rfind(wasp::lsp::m_uri_prefix, 0) == 0)
    parse_file_path.erase(0, std::string(wasp::lsp::m_uri_prefix).size());

  // copy parent application parameters and modify to set up input check
  InputParameters app_params = _moose_app.parameters();
  app_params.set<std::vector<std::string>>("input_file") = {parse_file_path};
  app_params.set<std::string>("_input_text") = document_text;
  app_params.set<bool>("check_input") = true;
  app_params.set<bool>("error_unused") = true;
  app_params.set<bool>("error") = true;
  app_params.set<bool>("error_deprecated") = true;
  app_params.set<std::string>("color") = "off";
  app_params.set<bool>("disable_perf_graph_live") = true;
  app_params.remove("language_server");

  // turn output off so input check application does not affect messages
  std::streambuf * cached_output_buffer = Moose::out.rdbuf(nullptr);

  // create new application with parameters modified for input check run
  _check_app = AppFactory::instance().createShared(
      _moose_app.type(), _moose_app.name(), app_params, _moose_app.getCommunicator()->get());

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

    std::istringstream caught_msg(err.what());

    // walk over caught message line by line adding each as a diagnostic
    for (std::string error_line; std::getline(caught_msg, error_line);)
    {
      // skip over lines that are blank or consist totally of whitespace
      if (error_line.find_first_not_of(" \t") == std::string::npos)
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

      // build zero based line and column diagnostic and add to the list
      diagnosticsList.push_back(wasp::DataObject());
      wasp::DataObject * diagnostic = diagnosticsList.back().to_object();
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

  // turn output back on since it was turned off and input check is done
  Moose::out.rdbuf(cached_output_buffer);

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
  // add only root level blocks to completion list when parser root is null
  if (!_check_app || !_check_app->parser()._root ||
      _check_app->parser()._root->getNodeView().is_null())
    return addSubblocksToList(completionItems, "/", line, character);

  // find hit node for zero based request line and column number from input
  wasp::HITNodeView view_root = _check_app->parser()._root->getNodeView();
  wasp::HITNodeView request_context =
      wasp::findNodeUnderLineColumn(view_root, line + 1, character + 1);

  // get object context and value of type parameter for request if provided
  wasp::HITNodeView object_context = request_context;
  while (object_context.type() != wasp::OBJECT && object_context.has_parent())
    object_context = object_context.parent();
  const std::string & object_path = object_context.path();
  wasp::HITNodeView type_node = object_context.first_child_by_name("type");
  const std::string & object_type = type_node.is_null() ? "" : type_node.last_as_string();

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

  // use context of request to pick types of items added to completion list
  if (request_context.type() == wasp::OBJECT || request_context.type() == wasp::DOCUMENT_ROOT)
  {
    // add gathered parameters to completion list at request input location
    pass &= addParametersToList(completionItems, valid_params, existing_params, line, character);

    // add all valid subblock objects to completion list for request object
    pass &= addSubblocksToList(completionItems, object_path, line, character);
  }
  else if (request_context.type() == wasp::VALUE)
  {
    // get name of parameter in input and zero based line and column ranges
    std::string param_name = request_context.has_parent() ? request_context.parent().name() : "";
    int replace_line_start = request_context.line() - 1;
    int replace_char_start = request_context.column() - 1;
    int replace_line_end = request_context.last_line() - 1;
    int replace_char_end = request_context.last_column();

    // add options to completion list for request value if parameter exists
    for (const auto & valid_params_iter : valid_params)
    {
      const std::string & valid_param_name = valid_params_iter.first;

      if (param_name == valid_param_name)
      {
        pass &= addValuesToList(completionItems,
                                valid_params,
                                existing_subblocks,
                                param_name,
                                obj_act_tasks,
                                replace_line_start,
                                replace_char_start,
                                replace_line_end,
                                replace_char_end);
        break;
      }
    }
  }

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
  valid_params = Parser::validParams();
  getActionParameters(valid_params, object_path, obj_act_tasks);
  getObjectParameters(valid_params, object_type, obj_act_tasks);
}

void
MooseServer::getActionParameters(InputParameters & valid_params,
                                 const std::string & object_path,
                                 std::set<std::string> & obj_act_tasks)
{
  Syntax & syntax = _check_app->syntax();
  ActionFactory & action_factory = _check_app->getActionFactory();

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
  Syntax & syntax = _check_app->syntax();
  Factory & factory = _check_app->getFactory();

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
    if (object_params.have_parameter<std::string>("_moose_base"))
    {
      const std::string & moose_base = object_params.get<std::string>("_moose_base");

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
                                 int request_line,
                                 int request_char)
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

    // process parameter description and type to use in input default value
    std::string dirty_type = valid_params.type(param_name);
    std::string clean_type = MooseUtils::prettyCppType(dirty_type);
    std::string basic_type = JsonSyntaxTree::basicCppType(clean_type);
    bool is_require = valid_params.isParamRequired(param_name);
    std::string doc_string =
        (is_require ? "(REQUIRED) " : "           ") + valid_params.getDocString(param_name);
    MooseUtils::escape(doc_string);

    // use basic type to decide if parameter is array and quotes are needed
    std::string array_quote = basic_type.compare(0, 6, "Array:") == 0 ? "'" : "";

    // remove any array prefixes from basic type string and leave base type
    pcrecpp::RE("(Array:)*(.*)").GlobalReplace("\\2", &basic_type);

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

    // otherwise if parameter is enum then use first item for default value
    if (default_value.empty())
    {
      std::map<std::string, std::string> options_and_descs;

      if (valid_params.have_parameter<MooseEnum>(param_name))
        getEnumsAndDocs(valid_params.get<MooseEnum>(param_name), options_and_descs);
      else if (valid_params.have_parameter<MultiMooseEnum>(param_name))
        getEnumsAndDocs(valid_params.get<MultiMooseEnum>(param_name), options_and_descs);
      else if (valid_params.have_parameter<ExecFlagEnum>(param_name))
        getEnumsAndDocs(valid_params.get<ExecFlagEnum>(param_name), options_and_descs);
      else if (valid_params.have_parameter<std::vector<MooseEnum>>(param_name))
        getEnumsAndDocs(valid_params.get<std::vector<MooseEnum>>(param_name)[0], options_and_descs);

      if (!options_and_descs.empty())
        default_value = options_and_descs.begin()->first;
    }

    // otherwise use parameter basic type to pick appropriate default value
    if (default_value.empty())
    {
      if (basic_type == "Boolean")
        default_value = "true";
      else if (basic_type == "Integer")
        default_value = "0";
      else if (basic_type == "Real")
        default_value = "0.0";
      else
        default_value = "value";
    }

    // switch 1 to true or 0 to false if boolean parameter as default value
    if (basic_type == "Boolean" && default_value == "1")
      default_value = "true";
    else if (basic_type == "Boolean" && default_value == "0")
      default_value = "false";

    // finally build full insertion from parameter name, quote, and default
    std::string embed_text = param_name + " = " + array_quote + default_value + array_quote;

    // add parameter label, insert text, and description to completion list
    completionItems.push_back(wasp::DataObject());
    wasp::DataObject * item = completionItems.back().to_object();
    pass &= wasp::lsp::buildCompletionObject(*item,
                                             errors,
                                             param_name,
                                             request_line,
                                             request_char,
                                             request_line,
                                             request_char,
                                             embed_text,
                                             1,
                                             "",
                                             doc_string,
                                             false,
                                             false);
  }

  return pass;
}

bool
MooseServer::addSubblocksToList(wasp::DataArray & completionItems,
                                const std::string & object_path,
                                int request_line,
                                int request_char)
{
  Syntax & syntax = _check_app->syntax();

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
    for (const auto & subblock_name : _syntax_to_subblocks[registered_syntax])
    {
      std::string doc_string;
      std::string embed_text;

      // customize description and insert text for star and named subblocks
      if (subblock_name == "*")
      {
        doc_string = "           custom user named block";
        embed_text = "[block_name]\n  \n[]";
      }
      else
      {
        doc_string = "           application named block";
        embed_text = "[" + subblock_name + "]\n  \n[]";
      }

      // add subblock name, insert text, and description to completion list
      completionItems.push_back(wasp::DataObject());
      wasp::DataObject * item = completionItems.back().to_object();
      pass &= wasp::lsp::buildCompletionObject(*item,
                                               errors,
                                               subblock_name,
                                               request_line,
                                               request_char,
                                               request_line,
                                               request_char,
                                               embed_text,
                                               1,
                                               "",
                                               doc_string,
                                               false,
                                               false);
    }
  }

  return pass;
}

bool
MooseServer::addValuesToList(wasp::DataArray & completionItems,
                             const InputParameters & valid_params,
                             const std::set<std::string> & existing_subblocks,
                             const std::string & param_name,
                             const std::set<std::string> & obj_act_tasks,
                             int replace_line_start,
                             int replace_char_start,
                             int replace_line_end,
                             int replace_char_end)
{
  Syntax & syntax = _check_app->syntax();
  Factory & factory = _check_app->getFactory();

  // get clean type for path associations and basic type for boolean values
  std::string dirty_type = valid_params.type(param_name);
  std::string clean_type = MooseUtils::prettyCppType(dirty_type);
  std::string basic_type = JsonSyntaxTree::basicCppType(clean_type);

  // remove any array prefixes from basic type string and replace with base
  pcrecpp::RE("(Array:)*(.*)").GlobalReplace("\\2", &basic_type);

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

      // check if object has registered base parameter that can be verified
      if (!object_params.have_parameter<std::string>("_moose_base"))
        continue;
      const std::string & moose_base = object_params.get<std::string>("_moose_base");

      // walk over gathered MooseObjectAction tasks and add if base matches
      for (const auto & obj_act_task : obj_act_tasks)
      {
        if (!syntax.verifyMooseObjectTask(moose_base, obj_act_task))
          continue;
        std::string type_description = object_params.getClassDescription();
        MooseUtils::escape(type_description);
        options_and_descs[object_name] = type_description;
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

    // prepare clean cpp type string to be used for key finding input paths
    pcrecpp::RE(".+<([A-Za-z0-9_' ':]*)>.*").GlobalReplace("\\1", &clean_type);

    // check for input lookup paths that are associated with parameter type
    const auto & input_path_iter = _type_to_input_paths.find(clean_type);

    if (input_path_iter != _type_to_input_paths.end())
    {
      wasp::HITNodeView view_root = _check_app->parser()._root->getNodeView();

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

  bool pass = true;

  // walk over pairs of options with descriptions and build completion list
  for (const auto & option_and_desc : options_and_descs)
  {
    const std::string & option = option_and_desc.first;
    const std::string & dscrpt = option_and_desc.second;

    // add option name, insertion range, and description to completion list
    completionItems.push_back(wasp::DataObject());
    wasp::DataObject * item = completionItems.back().to_object();
    pass &= wasp::lsp::buildCompletionObject(*item,
                                             errors,
                                             option,
                                             replace_line_start,
                                             replace_char_start,
                                             replace_line_end,
                                             replace_char_end,
                                             option,
                                             1,
                                             "",
                                             dscrpt,
                                             false,
                                             false);
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
MooseServer::gatherDocumentDefinitionLocations(wasp::DataArray & /* definitionLocations */,
                                               int /* line */,
                                               int /* character */)
{
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
  bool pass = true;

  // TODO - hook up this capability by adding server specific logic here

  return pass;
}

bool
MooseServer::gatherDocumentFormattingTextEdits(wasp::DataArray & /* formattingTextEdits */,
                                               int /* tab_size */,
                                               bool /* insert_spaces */)
{
  bool pass = true;

  // TODO - hook up this capability by adding server specific logic here

  return pass;
}

bool
MooseServer::gatherDocumentSymbols(wasp::DataArray & documentSymbols)
{
  // return prior to starting document symbol tree when parser root is null
  if (!_check_app || !_check_app->parser()._root ||
      _check_app->parser()._root->getNodeView().is_null())
    return true;

  wasp::HITNodeView view_root = _check_app->parser()._root->getNodeView();

  bool pass = true;

  // walk over all children of root node context and build document symbols
  for (auto itr = view_root.begin(); itr != view_root.end(); itr.next())
  {
    wasp::HITNodeView view_child = itr.get();

    // get child name / detail / line / column / last_line / last_column
    std::string name = view_child.name();
    std::string detail = !view_child.first_child_by_name("type").is_null()
                             ? view_child.first_child_by_name("type").last_as_string()
                             : "";
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

  // walk over all children of this node context and build document symbols
  for (auto itr = view_parent.begin(); itr != view_parent.end(); itr.next())
  {
    wasp::HITNodeView view_child = itr.get();

    // get child name / detail / line / column / last_line / last_column
    std::string name = view_child.name();
    std::string detail = !view_child.first_child_by_name("type").is_null()
                             ? view_child.first_child_by_name("type").last_as_string()
                             : "";
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
