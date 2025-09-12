//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseUtils.h"
#include "InputParameters.h"
#include "ActionFactory.h"
#include "Action.h"
#include "Factory.h"
#include "Parser.h"
#include "MooseObjectAction.h"
#include "AddActionComponentAction.h"
#include "ActionWarehouse.h"
#include "EmptyAction.h"
#include "MooseApp.h"
#include "GlobalParamsAction.h"
#include "SyntaxTree.h"
#include "InputFileFormatter.h"
#include "YAMLFormatter.h"
#include "MooseTypes.h"
#include "CommandLine.h"
#include "JsonSyntaxTree.h"
#include "Syntax.h"
#include "ParameterRegistry.h"

#include "libmesh/parallel.h"
#include "libmesh/fparser.hh"

#include <string>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <filesystem>

namespace Moose
{

bool
isSectionActive(const hit::Node & node)
{
  const hit::Node * n = &node;
  while (n)
  {
    const auto section = n->parent();
    if (section)
    {
      // only check current level, not nested ones
      if (const auto active = section->find("active");
          active && active->type() == hit::NodeType::Field && active->parent() == section)
      {
        const auto vars = active->param<std::vector<std::string>>();
        return std::find_if(vars.begin(),
                            vars.end(),
                            [&n](const auto & var)
                            { return n->path() == hit::pathNorm(var); }) != vars.end();
      }
      // only check current level, not nested ones
      if (const auto inactive = section->find("inactive");
          inactive && inactive->type() == hit::NodeType::Field && inactive->parent() == section)
      {
        const auto vars = inactive->param<std::vector<std::string>>();
        return std::find_if(vars.begin(),
                            vars.end(),
                            [&n](const auto & var)
                            { return n->path() == hit::pathNorm(var); }) == vars.end();
      }
    }
    n = section;
  }
  return true;
}

std::vector<std::string>
findSimilar(const std::string & param, const std::vector<std::string> & options)
{
  std::vector<std::string> candidates;
  if (options.size() == 0)
    return candidates;

  int mindist = MooseUtils::levenshteinDist(options[0], param);
  for (const auto & opt : options)
  {
    const int dist = MooseUtils::levenshteinDist(opt, param);
    // magic number heuristics to get similarity distance cutoff
    const int dist_cutoff = 1 + param.size() / 5;
    if (dist > dist_cutoff || dist > mindist)
      continue;

    if (dist < mindist)
    {
      mindist = dist;
      candidates.clear();
    }
    candidates.push_back(opt);
  }
  return candidates;
}

Builder::Builder(MooseApp & app, ActionWarehouse & action_wh, Parser & parser)
  : ConsoleStreamInterface(app),
    _app(app),
    _factory(app.getFactory()),
    _action_wh(action_wh),
    _action_factory(app.getActionFactory()),
    _syntax(_action_wh.syntax()),
    _parser(parser),
    _root(_parser.getRoot()),
    _syntax_formatter(nullptr)
{
}

Builder::~Builder() {}

InputParameters
Builder::validParams()
{
  InputParameters params = emptyInputParameters();

  /**
   * Add the "active" and "inactive" parameters so that all blocks in the input file
   * can selectively create lists of active/inactive sub-blocks.
   */
  params.addParam<std::vector<std::string>>(
      "active",
      std::vector<std::string>({"__all__"}),
      "If specified only the blocks named will be visited and made active");
  params.addParam<std::vector<std::string>>(
      "inactive",
      std::vector<std::string>(),
      "If specified blocks matching these identifiers will be skipped.");

  return params;
}

std::vector<std::string>
Builder::listValidParams(std::string & section_name)
{
  bool dummy;
  std::string registered_identifier = _syntax.isAssociated(section_name, &dummy);
  auto iters = _syntax.getActions(registered_identifier);

  std::vector<std::string> paramlist;
  for (auto it = iters.first; it != iters.second; ++it)
  {
    auto params = _action_factory.getValidParams(it->second._action);
    for (const auto & it : params)
      paramlist.push_back(it.first);
  }
  return paramlist;
}

void
UnusedWalker::walk(const std::string & fullpath, const std::string & nodename, hit::Node * n)
{
  // the line() > 0 check allows us to skip nodes that were merged into this tree (i.e. CLI
  // args) because their unused params are checked+reported independently of the ones in the
  // main tree.
  if (!_used.count(fullpath) && nodename != "active" && nodename != "inactive" &&
      isSectionActive(*n) && n->line() > 0)
  {
    auto section_name = fullpath.substr(0, fullpath.rfind("/"));
    const auto paramlist = _builder.listValidParams(section_name);
    const auto candidates = findSimilar(nodename, paramlist);
    if (candidates.size() > 0)
      errors.emplace_back(
          "unused parameter '" + fullpath + "'; did you mean '" + candidates[0] + "'?", n);
    else
      errors.emplace_back("unused parameter '" + fullpath + "'", n);
  }
}

std::string
Builder::getPrimaryFileName(bool strip_leading_path) const
{
  const auto path = _parser.getLastInputFilePath();
  return (strip_leading_path ? path.filename() : std::filesystem::absolute(path)).string();
}

void
Builder::walkRaw(std::string /*fullpath*/, std::string /*nodepath*/, hit::Node * n)
{
  InputParameters active_list_params = Action::validParams();
  InputParameters params = EmptyAction::validParams();

  const std::string section_name = n->fullpath();
  const std::string curr_identifier = n->fullpath();

  // Before we retrieve any actions or build any objects, make sure that the section they are in
  // is active
  if (!isSectionActive(*n))
    return;

  // Extract the block parameters before constructing the action
  // There may be more than one Action registered for a given section in which case we need to
  // build them all
  bool is_parent;
  std::string registered_identifier = _syntax.isAssociated(section_name, &is_parent);

  // Make sure at least one action is associated with the current identifier
  if (const auto [begin, end] = _syntax.getActions(registered_identifier); begin == end)
  {
    _errors.emplace_back(
        "section '[" + curr_identifier +
            "]' does not have an associated Action; you likely misspelled the Action/section name "
            "or the app you are running does not support this Action/syntax",
        n);
    return;
  }

  // The DynamicObjecRegistrationAction changes the action multimap and would invalidate the
  // iterators returned by _syntax.getActions, that's why we have to loop in this awkward way.
  std::set<const Syntax::ActionInfo *> processed_actions;
  while (true)
  {
    // search for an unprocessed action
    auto [begin, end] = _syntax.getActions(registered_identifier);
    auto it = begin;
    for (; it != end && processed_actions.count(&it->second); ++it)
      ;

    // no more unprocessed actions
    if (it == end)
      break;

    // mark action as processed
    processed_actions.insert(&it->second);

    if (is_parent)
      continue;
    if (_syntax.isDeprecatedSyntax(registered_identifier))
      mooseDeprecated(
          hit::errormsg(n, _syntax.deprecatedActionSyntaxMessage(registered_identifier)));

    params = _action_factory.getValidParams(it->second._action);
    params.set<ActionWarehouse *>("awh") = &_action_wh;
    params.setHitNode(*n, {});

    extractParams(n, params);

    // Add the parsed syntax to the parameters object for consumption by the Action
    params.set<std::string>("task") = it->second._task;
    params.set<std::string>("registered_identifier") = registered_identifier;

    if (!(params.have_parameter<bool>("isObjectAction") && params.get<bool>("isObjectAction")))
      params.set<std::vector<std::string>>("control_tags")
          .push_back(MooseUtils::baseName(curr_identifier));

    // Create the Action
    std::shared_ptr<Action> action_obj =
        _action_factory.create(it->second._action, curr_identifier, params);

    {
      // extract the MooseObject params if necessary
      std::shared_ptr<MooseObjectAction> object_action =
          std::dynamic_pointer_cast<MooseObjectAction>(action_obj);
      if (object_action)
      {
        auto & object_params = object_action->getObjectParams();
        object_params.setHitNode(*n, {});
        extractParams(n, object_params);
        object_params.set<std::vector<std::string>>("control_tags")
            .push_back(MooseUtils::baseName(curr_identifier));
      }
      // extract the Component params if necessary
      std::shared_ptr<AddActionComponentAction> component_action =
          std::dynamic_pointer_cast<AddActionComponentAction>(action_obj);
      if (component_action)
      {
        auto & component_params = component_action->getComponentParams();
        component_params.setHitNode(*n, {});
        extractParams(n, component_params);
        component_params.set<std::vector<std::string>>("control_tags")
            .push_back(MooseUtils::baseName(curr_identifier));
      }
    }

    // add it to the warehouse
    _action_wh.addActionBlock(action_obj);
  }
}

void
Builder::walk(const std::string & fullpath, const std::string & nodepath, hit::Node * n)
{
  // skip sections that were manually processed first.
  if (std::find(_secs_need_first.begin(), _secs_need_first.end(), nodepath) !=
      _secs_need_first.end())
    return;

  walkRaw(fullpath, nodepath, n);
}

void
Builder::build()
{
  // Pull in extracted variables from the parser (fparse stuff)
  _extracted_vars = _parser.getExtractedVars();

  // There are a few order dependent actions that have to be built first in
  // order for the parser and application to function properly
  const auto need_action_syntax_first = [this](const auto & action_name)
  {
    const auto syntax = _syntax.getSyntaxByAction(action_name);
    mooseAssert(syntax.size(), "Empty syntax");
    std::copy(syntax.begin(), syntax.end(), std::back_inserter(_secs_need_first));
  };

  // SetupDebugAction: This action can contain an option for monitoring the
  // parser progress. It must be parsed first to capture all of the parsing
  // output progress.
  need_action_syntax_first("SetupDebugAction");

  // GlobalParamsAction: This action is checked during the parameter extraction
  // routines of all subsequent blocks. It must be parsed early since it must
  // exist during subsequent parameter extraction.
  need_action_syntax_first("GlobalParamsAction");

  // DynamicObjectRegistration: This action must be built before any MooseObjectActions
  // are built. This is because we retrieve valid parameters from the Factory
  // during parse time. Objects must be registered before validParameters can be retrieved.
  need_action_syntax_first("DynamicObjectRegistrationAction");

  // Walk all the sections extracting paramters from each into InputParameters objects
  for (const auto & sec : _secs_need_first)
    if (auto n = _root.find(sec))
      walkRaw(n->parent()->fullpath(), n->path(), n);

  // Walk for the remainder
  _root.walk(this, hit::NodeType::Section);

  // Warn for all deprecated parameters together
  if (_deprecated_params.size())
  {
    std::vector<std::string> messages;
    for (const auto & key_message_pair : _deprecated_params)
      messages.push_back(key_message_pair.second);
    const auto message = MooseUtils::stringJoin(messages, "\n\n");

    const auto current_show_trace = Moose::show_trace;
    Moose::show_trace = false;
    moose::internal::mooseDeprecatedStream(Moose::out, false, true, message + "\n\n");
    Moose::show_trace = current_show_trace;
  }

  if (_errors.size())
    _parser.parseError(_errors);
}

// Checks the input and the way it has been used and emits any errors/warnings.
// This has to be a separate function because for we don't know if some parameters were unused
// until all the multiapps/subapps have been fully initialized - which isn't complete until
// *after* all the other member functions on Parser have been run.  So this is here to be
// externally called at the right time.
void
Builder::errorCheck(const Parallel::Communicator & comm, bool warn_unused, bool err_unused)
{
  // Nothing to do here
  if (!warn_unused && !err_unused)
    return;

  std::vector<hit::ErrorMessage> messages;

  {
    UnusedWalker uw(_extracted_vars, *this);
    _parser.getCommandLineRoot().walk(&uw);
    Parser::appendErrorMessages(messages, uw.errors);
  }

  {
    UnusedWalker uw(_extracted_vars, *this);
    _root.walk(&uw);
    Parser::appendErrorMessages(messages, uw.errors);
  }

  for (const auto & arg : _app.commandLine()->unusedHitParams(comm))
    messages.emplace_back("unused command line parameter '" + arg + "'");

  if (messages.size())
  {
    const auto message = _parser.joinErrorMessages(messages);
    if (warn_unused)
      mooseUnused(message);
    if (err_unused)
    {
      if (_parser.getThrowOnError())
        _parser.parseError(messages);
      else
        mooseError(
            message +
            "\n\nAppend --allow-unused (or -w) on the command line to ignore unused parameters.");
    }
  }
}

void
Builder::initSyntaxFormatter(SyntaxFormatterType type, bool dump_mode)
{
  switch (type)
  {
    case INPUT_FILE:
      _syntax_formatter = std::make_unique<InputFileFormatter>(dump_mode);
      break;
    case YAML:
      _syntax_formatter = std::make_unique<YAMLFormatter>(dump_mode);
      break;
    default:
      mooseError("Unrecognized Syntax Formatter requested");
      break;
  }
}

void
Builder::buildJsonSyntaxTree(JsonSyntaxTree & root) const
{
  std::vector<std::pair<std::string, Syntax::ActionInfo>> all_names;

  for (const auto & iter : _syntax.getAssociatedTypes())
    root.addSyntaxType(iter.first, iter.second);

  // Build a list of all the actions appearing in the syntax
  for (const auto & iter : _syntax.getAssociatedActions())
  {
    Syntax::ActionInfo act_info = iter.second;
    /**
     * If the task is nullptr that means we need to figure out which task goes with this syntax for
     * the purpose of building the Moose Object part of the tree. We will figure this out by asking
     * the ActionFactory for the registration info.
     */
    if (act_info._task == "")
      act_info._task = _action_factory.getTaskName(act_info._action);

    all_names.push_back(std::make_pair(iter.first, act_info));
  }

  // Cache to avoid building an object's parameters multiple times
  const auto object_params = buildRegisteredObjectParams();

  // Add all the actions to the JSON tree, except for ActionComponents (below)
  for (const auto & act_names : all_names)
  {
    const auto & act_info = act_names.second;
    const std::string & action = act_info._action;
    const std::string & task = act_info._task;
    const std::string syntax = act_names.first;
    const InputParameters action_obj_params = _action_factory.getValidParams(action);
    bool params_added = root.addParameters("",
                                           syntax,
                                           false,
                                           action,
                                           true,
                                           action_obj_params,
                                           _syntax.getLineInfo(syntax, action, ""),
                                           "");

    if (params_added)
    {
      auto tasks = _action_factory.getTasksByAction(action);
      for (auto & t : tasks)
      {
        auto info = _action_factory.getLineInfo(action, t);
        root.addActionTask(syntax, action, t, info);
      }
    }

    /**
     * We need to see if this action is inherited from MooseObjectAction. If it is, then we will
     * loop over all the Objects in MOOSE's Factory object to print them out if they have associated
     * bases matching the current task.
     */
    if (action_obj_params.have_parameter<bool>("isObjectAction") &&
        action_obj_params.get<bool>("isObjectAction"))
    {
      for (const auto & [moose_obj_name, moose_obj_params] : object_params)
      {

        // Now that we know that this is a MooseObjectAction we need to see if it has been
        // restricted
        // in any way by the user.
        const std::vector<std::string> & buildable_types = action_obj_params.getBuildableTypes();

        // See if the current Moose Object syntax belongs under this Action's block
        if ((buildable_types.empty() || // Not restricted
             std::find(buildable_types.begin(), buildable_types.end(), moose_obj_name) !=
                 buildable_types.end()) && // Restricted but found
            moose_obj_params.hasBase() &&  // Has a registered base
            _syntax.verifyMooseObjectTask(moose_obj_params.getBase(),
                                          task) &&          // and that base is associated
            action_obj_params.mooseObjectSyntaxVisibility() // and the Action says it's visible
        )
        {
          std::string name;
          size_t pos = 0;
          bool is_action_params = false;
          bool is_type = false;
          if (syntax[syntax.size() - 1] == '*')
          {
            pos = syntax.size();

            if (!action_obj_params.collapseSyntaxNesting())
              name = syntax.substr(0, pos - 1) + moose_obj_name;
            else
            {
              name = syntax.substr(0, pos - 1) + "/<type>/" + moose_obj_name;
              is_action_params = true;
            }
          }
          else
          {
            name = syntax + "/<type>/" + moose_obj_name;
            is_type = true;
          }

          auto lineinfo = _factory.getLineInfo(moose_obj_name);
          std::string classname = _factory.associatedClassName(moose_obj_name);
          root.addParameters(syntax,
                             name,
                             is_type,
                             moose_obj_name,
                             is_action_params,
                             moose_obj_params,
                             lineinfo,
                             classname);
        }
      }

      // Same thing for ActionComponents, which, while they are not MooseObjects, should behave
      // similarly syntax-wise
      if (syntax != "ActionComponents/*")
        continue;

      auto iters = _action_factory.getActionsByTask("list_component");

      for (auto it = iters.first; it != iters.second; ++it)
      {
        // Get the name and parameters
        const auto component_name = it->second;
        auto component_params = _action_factory.getValidParams(component_name);

        // We currently do not have build-type restrictions on this action that adds
        // action-components

        // See if the current Moose Object syntax belongs under this Action's block
        if (action_obj_params.mooseObjectSyntaxVisibility() // and the Action says it's visible
        )
        {
          // The logic for Components is a little simpler here for now because syntax like
          // Executioner/TimeIntegrator/type= do not exist for components
          std::string name;
          if (syntax[syntax.size() - 1] == '*')
          {
            size_t pos = syntax.size();
            name = syntax.substr(0, pos - 1) + component_name;
          }
          component_params.set<std::string>("type") = component_name;

          auto lineinfo = _action_factory.getLineInfo(component_name, "list_component");
          // We add the parameters as for an object, because we want to fit them to be
          // added to json["AddActionComponentAction"]["subblock_types"]
          root.addParameters(syntax,
                             /*syntax_path*/ name,
                             /*is_type*/ false,
                             "AddActionComponentAction",
                             /*is_action=*/false,
                             component_params,
                             lineinfo,
                             component_name);
        }
      }
    }
  }
  root.addGlobal();
}

void
Builder::buildFullTree(const std::string & search_string)
{
  std::vector<std::pair<std::string, Syntax::ActionInfo>> all_names;

  for (const auto & iter : _syntax.getAssociatedActions())
  {
    Syntax::ActionInfo act_info = iter.second;
    /**
     * If the task is nullptr that means we need to figure out which task goes with this syntax for
     * the purpose of building the Moose Object part of the tree. We will figure this out by asking
     * the ActionFactory for the registration info.
     */
    if (act_info._task == "")
      act_info._task = _action_factory.getTaskName(act_info._action);

    all_names.push_back(std::pair<std::string, Syntax::ActionInfo>(iter.first, act_info));
  }

  // Cache to avoid building an object's parameters multiple times
  const auto object_params = buildRegisteredObjectParams();

  for (const auto & act_names : all_names)
  {
    InputParameters action_obj_params = _action_factory.getValidParams(act_names.second._action);
    _syntax_formatter->insertNode(
        act_names.first, act_names.second._action, true, &action_obj_params);

    const std::string & task = act_names.second._task;
    std::string act_name = act_names.first;

    /**
     * We need to see if this action is inherited from MooseObjectAction. If it is, then we will
     * loop over all the Objects in MOOSE's Factory object to print them out if they have associated
     * bases matching the current task.
     */
    if (action_obj_params.have_parameter<bool>("isObjectAction") &&
        action_obj_params.get<bool>("isObjectAction"))
    {
      for (const auto & [moose_obj_name, moose_obj_params] : object_params)
      {
        /**
         * Now that we know that this is a MooseObjectAction we need to see if it has been
         * restricted in any way by the user.
         */
        const std::vector<std::string> & buildable_types = action_obj_params.getBuildableTypes();

        // See if the current Moose Object syntax belongs under this Action's block
        if ((buildable_types.empty() || // Not restricted
             std::find(buildable_types.begin(), buildable_types.end(), moose_obj_name) !=
                 buildable_types.end()) && // Restricted but found
            moose_obj_params.hasBase() &&  // Has a registered base
            _syntax.verifyMooseObjectTask(moose_obj_params.getBase(),
                                          task) &&          // and that base is associated
            action_obj_params.mooseObjectSyntaxVisibility() // and the Action says it's visible
        )
        {
          std::string name;
          size_t pos = 0;
          bool is_action_params = false;
          if (act_name[act_name.size() - 1] == '*')
          {
            pos = act_name.size();

            if (!action_obj_params.collapseSyntaxNesting())
              name = act_name.substr(0, pos - 1) + moose_obj_name;
            else
            {
              name = act_name.substr(0, pos - 1) + "/<type>/" + moose_obj_name;
              is_action_params = true;
            }
          }
          else
          {
            name = act_name + "/<type>/" + moose_obj_name;
          }

          _syntax_formatter->insertNode(name, moose_obj_name, is_action_params, &moose_obj_params);
        }
      }
    }
  }

  // Do not change to _console, we need this printed to the stdout in all cases
  Moose::out << _syntax_formatter->print(search_string) << std::flush;
}

void
Builder::extractParams(const hit::Node * const section_node, InputParameters & p)
{
  if (section_node)
    mooseAssert(section_node->type() == hit::NodeType::Section, "Node type should be a section");

  for (const auto & [name, par_unique_ptr] : p)
  {
    if (p.shouldIgnore(name))
      continue;

    const hit::Node * param_node = nullptr;

    for (const auto & param_name : p.paramAliases(name))
    {
      // Check for parameters under the given section, if a section
      // node was provided
      if (section_node)
      {
        if (const auto section_param_node = section_node->find(param_name);
            section_param_node && section_param_node->type() == hit::NodeType::Field &&
            section_param_node->parent() == section_node)
          param_node = section_param_node;
      }
      // No node found within the given section, check [GlobalParams]
      if (!param_node && queryGlobalParamsNode())
      {
        if (const auto global_node = queryGlobalParamsNode()->find(param_name);
            global_node && global_node->type() == hit::NodeType::Field &&
            global_node->parent() == queryGlobalParamsNode())
        {
          mooseAssert(isGlobal(*global_node), "Could not detect global-ness");
          param_node = global_node;
        }
      }

      // Found it
      if (param_node)
      {
        const auto fullpath = param_node->fullpath();
        p.setHitNode(param_name, *param_node, {});
        p.set_attributes(param_name, false);
        _extracted_vars.insert(fullpath);

        const auto global = isGlobal(*param_node);

        // Check for deprecated parameters if the parameter is not a global param
        if (!global)
          if (const auto deprecated_message = p.queryDeprecatedParamMessage(param_name))
          {
            std::string key = "";
            if (const auto object_type_ptr = p.queryObjectType())
              key += *object_type_ptr + "_";
            key += param_name;
            _deprecated_params.emplace(key, *deprecated_message);
          }

        // Private parameter, don't set
        if (p.isPrivate(param_name))
        {
          // Error if it isn't global, just once
          if (!global && std::find_if(_errors.begin(),
                                      _errors.end(),
                                      [&param_node](const auto & err)
                                      { return err.node == param_node; }) == _errors.end())
            _errors.emplace_back("parameter '" + fullpath + "' is private and cannot be set",
                                 param_node);
          continue;
        }

        // Set the value, capturing errors
        const auto param_field = dynamic_cast<const hit::Field *>(param_node);
        mooseAssert(param_field, "Is not a field");
        bool set_param = false;
        try
        {
          ParameterRegistry::get().set(*par_unique_ptr, *param_field);
          set_param = true;
        }
        catch (hit::Error & e)
        {
          _errors.emplace_back(e.message, param_node);
        }
        catch (std::exception & e)
        {
          _errors.emplace_back(e.what(), param_node);
        }

        // Break if we failed here and don't perform extra checks
        if (!set_param)
          break;

        // Special setup for vector<VariableName>
        if (auto cast_par = dynamic_cast<InputParameters::Parameter<std::vector<VariableName>> *>(
                par_unique_ptr.get()))
          if (const auto error = p.setupVariableNames(cast_par->set(), *param_node, {}))
            _errors.emplace_back(*error, param_node);

        // Possibly perform a range check if this parameter has one
        if (p.isRangeChecked(param_node->path()))
          if (const auto error = p.parameterRangeCheck(
                  *par_unique_ptr, param_node->fullpath(), param_node->path(), true))
            _errors.emplace_back(error->second, param_node);

        // Don't check the other alises since we've found it
        break;
      }
    }

    // Special casing when the parameter was not found
    if (!param_node)
    {
      // In the case where we have OutFileName but it wasn't actually found in the input filename,
      // we will populate it with the actual parsed filename which is available here in the
      // parser.
      if (auto out_par_ptr =
              dynamic_cast<InputParameters::Parameter<OutFileBase> *>(par_unique_ptr.get()))
      {
        const auto input_file_name = getPrimaryFileName();
        mooseAssert(input_file_name.size(), "Input Filename is empty");
        const auto pos = input_file_name.find_last_of('.');
        mooseAssert(pos != std::string::npos, "Unable to determine suffix of input file name");
        out_par_ptr->set() = input_file_name.substr(0, pos) + "_out";
        p.set_attributes(name, false);
      }
    }
  }

  // See if there are any auto build vectors that need to be created
  for (const auto & [param_name, base_name_num_repeat_pair] : p.getAutoBuildVectors())
  {
    const auto & [base_name, num_repeat] = base_name_num_repeat_pair;
    // We'll autogenerate values iff the requested vector is not valid but both the base and
    // number are valid
    if (!p.isParamValid(param_name) && p.isParamValid(base_name) && p.isParamValid(num_repeat))
    {
      const auto vec_size = p.get<unsigned int>(num_repeat);
      const std::string & name = p.get<std::string>(base_name);

      std::vector<VariableName> variable_names(vec_size);
      for (const auto i : index_range(variable_names))
      {
        std::ostringstream oss;
        oss << name << i;
        variable_names[i] = oss.str();
      }

      // Finally set the autogenerated vector into the InputParameters object
      p.set<std::vector<VariableName>>(param_name) = variable_names;
    }
  }
}

void
Builder::extractParams(const std::string & prefix, InputParameters & p)
{
  const auto node = _root.find(prefix);
  extractParams((node && node->type() == hit::NodeType::Section) ? node : nullptr, p);
}

bool
Builder::isGlobal(const hit::Node & node) const
{
  const auto global_params_node = queryGlobalParamsNode();
  return global_params_node && node.parent() == global_params_node;
}

const hit::Node *
Builder::queryGlobalParamsNode() const
{
  if (!_global_params_node)
  {
    const auto syntax = _syntax.getSyntaxByAction("GlobalParamsAction");
    mooseAssert(syntax.size() == 1, "Unexpected GlobalParamsAction syntax size");
    _global_params_node = _root.find(syntax.front());
  }
  return *_global_params_node;
}

std::vector<std::pair<std::string, const InputParameters>>
Builder::buildRegisteredObjectParams() const
{
  std::vector<std::pair<std::string, const InputParameters>> object_params;

  const auto & objects = _factory.registeredObjects();
  object_params.reserve(objects.size());

  for (const auto & [type, entry_ptr] : _factory.registeredObjects())
  {
    auto params = entry_ptr->buildParameters();
    params.set<std::string>("type") = type;
    object_params.emplace_back(type, std::move(params));
  }

  return object_params;
}

} // end of namespace Moose
