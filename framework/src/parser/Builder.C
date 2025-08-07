//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MooseUtils.h"
#include "MooseInit.h"
#include "InputParameters.h"
#include "ActionFactory.h"
#include "Action.h"
#include "Factory.h"
#include "Parser.h"
#include "MooseObjectAction.h"
#include "AddActionComponentAction.h"
#include "ActionWarehouse.h"
#include "EmptyAction.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "Executioner.h"
#include "MooseApp.h"
#include "MooseEnum.h"
#include "MultiMooseEnum.h"
#include "MultiApp.h"
#include "GlobalParamsAction.h"
#include "SyntaxTree.h"
#include "InputFileFormatter.h"
#include "YAMLFormatter.h"
#include "MooseTypes.h"
#include "CommandLine.h"
#include "JsonSyntaxTree.h"
#include "SystemInfo.h"
#include "MooseUtils.h"
#include "Conversion.h"
#include "Units.h"
#include "ActionComponent.h"
#include "Syntax.h"

#include "libmesh/parallel.h"
#include "libmesh/fparser.hh"

// Regular expression includes
#include "pcrecpp.h"

// C++ includes
#include <string>
#include <map>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <filesystem>

namespace Moose
{

bool
isSectionActive(std::string path, hit::Node * root)
{
  hit::Node * n = root->find(path);
  while (n)
  {
    hit::Node * section = n->parent();
    if (section)
    {
      auto actives = section->find("active");
      auto inactives = section->find("inactive");

      // only check current level, not nested ones
      if (actives && actives->type() == hit::NodeType::Field && actives->parent() == section)
      {
        auto vars = section->param<std::vector<std::string>>("active");
        bool have_var = false;
        for (auto & var : vars)
          if (n->path() == hit::pathNorm(var))
            have_var = true;
        if (!have_var)
          return false;
      }
      // only check current level, not nested ones
      if (inactives && inactives->type() == hit::NodeType::Field && inactives->parent() == section)
      {
        auto vars = section->param<std::vector<std::string>>("inactive");
        for (auto & var : vars)
          if (n->path() == hit::pathNorm(var))
            return false;
      }
    }
    n = section;
  }
  return true;
}

std::vector<std::string>
findSimilar(std::string param, std::vector<std::string> options)
{
  std::vector<std::string> candidates;
  if (options.size() == 0)
    return candidates;

  int mindist = MooseUtils::levenshteinDist(options[0], param);
  for (auto & opt : options)
  {
    int dist = MooseUtils::levenshteinDist(opt, param);
    // magic number heuristics to get similarity distance cutoff
    int dist_cutoff = 1 + param.size() / 5;
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
    _syntax_formatter(nullptr),
    _current_params(nullptr)
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
      isSectionActive(fullpath, n->root()) && n->line() > 0)
  {
    auto section_name = fullpath.substr(0, fullpath.rfind("/"));
    auto paramlist = _builder.listValidParams(section_name);
    auto candidates = findSimilar(nodename, paramlist);
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
  if (!isSectionActive(curr_identifier, &_root))
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

    extractParams(curr_identifier, params);

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
        extractParams(curr_identifier, object_params);
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
        extractParams(curr_identifier, component_params);
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
  for (auto & sec : _secs_need_first)
    if (nodepath == sec)
      return;
  walkRaw(fullpath, nodepath, n);
}

void
Builder::build()
{
  // Pull in extracted variables from the parser (fparse stuff)
  _extracted_vars = _parser.getExtractedVars();

  // There are a few order dependent actions that have to be built first in
  // order for the parser and application to function properly:
  //
  // SetupDebugAction: This action can contain an option for monitoring the parser progress. It must
  //                   be parsed first to capture all of the parsing output.
  //
  // GlobalParamsAction: This action is checked during the parameter extraction routines of all
  //                     subsequent blocks. It must be parsed early since it must exist during
  //                     subsequent parameter extraction.
  //
  // DynamicObjectRegistration: This action must be built before any MooseObjectActions are built.
  //                            This is because we retrieve valid parameters from the Factory
  //                            during parse time. Objects must be registered before
  //                            validParameters can be retrieved.
  auto syntax = _syntax.getSyntaxByAction("SetupDebugAction");
  std::copy(syntax.begin(), syntax.end(), std::back_inserter(_secs_need_first));

  syntax = _syntax.getSyntaxByAction("GlobalParamsAction");
  std::copy(syntax.begin(), syntax.end(), std::back_inserter(_secs_need_first));

  syntax = _syntax.getSyntaxByAction("DynamicObjectRegistrationAction");
  std::copy(syntax.begin(), syntax.end(), std::back_inserter(_secs_need_first));

  // Walk all the sections extracting paramters from each into InputParameters objects
  for (auto & sec : _secs_need_first)
    if (auto n = _root.find(sec))
      walkRaw(n->parent()->fullpath(), n->path(), n);

  // Walk for the remainder
  _root.walk(this, hit::NodeType::Section);

  // Warn for all deprecated parameters together
  if (_deprecated_params.size())
  {
    std::vector<std::string> messages;
    for (const auto & param_message_pair : _deprecated_params)
      messages.push_back(param_message_pair.second);
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

  // Add all the actions to the JSON tree, except for ActionComponents (below)
  for (const auto & act_names : all_names)
  {
    const auto & act_info = act_names.second;
    const std::string & action = act_info._action;
    const std::string & task = act_info._task;
    const std::string syntax = act_names.first;
    InputParameters action_obj_params = _action_factory.getValidParams(action);
    bool params_added = root.addParameters("",
                                           syntax,
                                           false,
                                           action,
                                           true,
                                           &action_obj_params,
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
      for (auto & [moose_obj_name, obj] : _factory.registeredObjects())
      {
        auto moose_obj_params = obj->buildParameters();
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
          moose_obj_params.set<std::string>("type") = moose_obj_name;

          auto lineinfo = _factory.getLineInfo(moose_obj_name);
          std::string classname = _factory.associatedClassName(moose_obj_name);
          root.addParameters(syntax,
                             name,
                             is_type,
                             moose_obj_name,
                             is_action_params,
                             &moose_obj_params,
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
                             &component_params,
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
      for (const auto & [moose_obj_name, obj] : _factory.registeredObjects())
      {
        auto moose_obj_params = obj->buildParameters();
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

          moose_obj_params.set<std::string>("type") = moose_obj_name;

          _syntax_formatter->insertNode(name, moose_obj_name, is_action_params, &moose_obj_params);
        }
      }
    }
  }

  // Do not change to _console, we need this printed to the stdout in all cases
  Moose::out << _syntax_formatter->print(search_string) << std::flush;
}

/**************************************************************************************************
 **************************************************************************************************
 *                                   Parameter Extraction Routines                                *
 **************************************************************************************************
 **************************************************************************************************/
using std::string;

// Template Specializations for retrieving special types from the input file
template <>
void Builder::setScalarParameter<RealVectorValue, RealVectorValue>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<RealVectorValue> * param,
    bool in_global,
    GlobalParamsAction * global_block);

template <>
void Builder::setScalarParameter<Point, Point>(const std::string & full_name,
                                               const std::string & short_name,
                                               InputParameters::Parameter<Point> * param,
                                               bool in_global,
                                               GlobalParamsAction * global_block);

template <>
void Builder::setScalarParameter<RealEigenVector, RealEigenVector>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<RealEigenVector> * param,
    bool in_global,
    GlobalParamsAction * global_block);

template <>
void Builder::setScalarParameter<RealEigenMatrix, RealEigenMatrix>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<RealEigenMatrix> * param,
    bool in_global,
    GlobalParamsAction * global_block);

template <>
void Builder::setScalarParameter<PostprocessorName, PostprocessorName>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<PostprocessorName> * param,
    bool in_global,
    GlobalParamsAction * global_block);

template <>
void
Builder::setScalarParameter<MooseEnum, MooseEnum>(const std::string & full_name,
                                                  const std::string & short_name,
                                                  InputParameters::Parameter<MooseEnum> * param,
                                                  bool in_global,
                                                  GlobalParamsAction * global_block);

template <>
void Builder::setScalarParameter<MultiMooseEnum, MultiMooseEnum>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<MultiMooseEnum> * param,
    bool in_global,
    GlobalParamsAction * global_block);

template <>
void Builder::setScalarParameter<ExecFlagEnum, ExecFlagEnum>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<ExecFlagEnum> * param,
    bool in_global,
    GlobalParamsAction * global_block);

template <>
void Builder::setScalarParameter<RealTensorValue, RealTensorValue>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<RealTensorValue> * param,
    bool in_global,
    GlobalParamsAction * global_block);

template <>
void Builder::setScalarParameter<ReporterName, std::string>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<ReporterName> * param,
    bool in_global,
    GlobalParamsAction * global_block);

// Vectors
template <>
void Builder::setVectorParameter<RealVectorValue, RealVectorValue>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<std::vector<RealVectorValue>> * param,
    bool in_global,
    GlobalParamsAction * global_block);

template <>
void
Builder::setVectorParameter<Point, Point>(const std::string & full_name,
                                          const std::string & short_name,
                                          InputParameters::Parameter<std::vector<Point>> * param,
                                          bool in_global,
                                          GlobalParamsAction * global_block);

template <>
void Builder::setVectorParameter<PostprocessorName, PostprocessorName>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<std::vector<PostprocessorName>> * param,
    bool in_global,
    GlobalParamsAction * global_block);

template <>
void Builder::setVectorParameter<MooseEnum, MooseEnum>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<std::vector<MooseEnum>> * param,
    bool in_global,
    GlobalParamsAction * global_block);

template <>
void Builder::setVectorParameter<MultiMooseEnum, MultiMooseEnum>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<std::vector<MultiMooseEnum>> * param,
    bool in_global,
    GlobalParamsAction * global_block);

template <>
void Builder::setVectorParameter<VariableName, VariableName>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<std::vector<VariableName>> * param,
    bool in_global,
    GlobalParamsAction * global_block);

template <>
void Builder::setVectorParameter<ReporterName, std::string>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<std::vector<ReporterName>> * param,
    bool in_global,
    GlobalParamsAction * global_block);

template <>
void Builder::setVectorParameter<CLIArgString, std::string>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<std::vector<CLIArgString>> * param,
    bool in_global,
    GlobalParamsAction * global_block);

template <>
void Builder::setDoubleIndexParameter<Point>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<std::vector<std::vector<Point>>> * param,
    bool in_global,
    GlobalParamsAction * global_block);

void
Builder::extractParams(const std::string & prefix, InputParameters & p)
{
  std::ostringstream error_stream;
  static const std::string global_params_task = "set_global_params";
  static const std::string global_params_block_name =
      _syntax.getSyntaxByAction("GlobalParamsAction").front();

  ActionIterator act_iter = _action_wh.actionBlocksWithActionBegin(global_params_task);
  GlobalParamsAction * global_params_block = nullptr;

  // We are grabbing only the first
  if (act_iter != _action_wh.actionBlocksWithActionEnd(global_params_task))
    global_params_block = dynamic_cast<GlobalParamsAction *>(*act_iter);

  // Set a pointer to the current InputParameters object being parsed so that it
  // can be referred to in the extraction routines
  _current_params = &p;
  for (const auto & it : p)
  {
    if (p.shouldIgnore(it.first))
      continue;

    const hit::Node * node = nullptr;
    bool found = false;
    bool in_global = false;

    const auto found_param =
        [this, &found, &node, &p](const std::string & param_name, const std::string & full_name)
    {
      found = true;
      p.setHitNode(param_name, *node, {});
      p.set_attributes(param_name, false);
      _extracted_vars.insert(full_name);
    };

    for (const auto & param_name : p.paramAliases(it.first))
    {
      std::string full_name = prefix + "/" + param_name;

      // Mark parameters appearing in the input file or command line
      if (node = _root.find(full_name); node && node->type() == hit::NodeType::Field)
      {
        found_param(param_name, full_name);

        // Store a deprecated warning if one exists and we haven't for this parameter
        // TODO: This is pretty bad and only lets us skip per parameter name...
        if (const auto deprecated_message = p.queryDeprecatedParamMessage(param_name))
          _deprecated_params.emplace(param_name, *deprecated_message);
      }
      // Wait! Check the GlobalParams section
      else if (global_params_block)
      {
        full_name = global_params_block_name + "/" + param_name;
        if (node = _root.find(full_name); node)
        {
          found_param(param_name, full_name);
          in_global = true;
        }
      }
      if (found)
      {

        if (p.isPrivate(param_name) && !in_global)
        {
          // Warn on private, just once
          if (std::find_if(_errors.begin(),
                           _errors.end(),
                           [&node](const auto & err) { return err.node == node; }) == _errors.end())
            _errors.emplace_back("parameter '" + full_name + "' is private and cannot be set",
                                 node);
          continue;
        }
        // avoid setting the parameter
        else if (p.isPrivate(param_name) && in_global)
          continue;

        auto & short_name = param_name;
        libMesh::Parameters::Value * par = MooseUtils::get(it.second);

#define setscalarvaltype(ptype, base, range)                                                       \
  else if (par->type() == demangle(typeid(ptype).name()))                                          \
      setScalarValueTypeParameter<ptype, range, base>(                                             \
          full_name,                                                                               \
          short_name,                                                                              \
          dynamic_cast<InputParameters::Parameter<ptype> *>(par),                                  \
          in_global,                                                                               \
          global_params_block,                                                                     \
          *node)
#define setscalar(ptype, base)                                                                     \
  else if (par->type() == demangle(typeid(ptype).name()))                                          \
      setScalarParameter<ptype, base>(full_name,                                                   \
                                      short_name,                                                  \
                                      dynamic_cast<InputParameters::Parameter<ptype> *>(par),      \
                                      in_global,                                                   \
                                      global_params_block)
#define setvector(ptype, base)                                                                     \
  else if (par->type() == demangle(typeid(std::vector<ptype>).name()))                             \
      setVectorParameter<ptype, base>(                                                             \
          full_name,                                                                               \
          short_name,                                                                              \
          dynamic_cast<InputParameters::Parameter<std::vector<ptype>> *>(par),                     \
          in_global,                                                                               \
          global_params_block)
#define setmap(key_type, mapped_type)                                                              \
  else if (par->type() == demangle(typeid(std::map<key_type, mapped_type>).name()))                \
      setMapParameter(                                                                             \
          full_name,                                                                               \
          short_name,                                                                              \
          dynamic_cast<InputParameters::Parameter<std::map<key_type, mapped_type>> *>(par),        \
          in_global,                                                                               \
          global_params_block)
#define setvectorvector(ptype)                                                                     \
  else if (par->type() == demangle(typeid(std::vector<std::vector<ptype>>).name()))                \
      setDoubleIndexParameter<ptype>(                                                              \
          full_name,                                                                               \
          short_name,                                                                              \
          dynamic_cast<InputParameters::Parameter<std::vector<std::vector<ptype>>> *>(par),        \
          in_global,                                                                               \
          global_params_block)
#define setvectorvectorvector(ptype)                                                               \
  else if (par->type() == demangle(typeid(std::vector<std::vector<std::vector<ptype>>>).name()))   \
      setTripleIndexParameter<ptype>(                                                              \
          full_name,                                                                               \
          short_name,                                                                              \
          dynamic_cast<                                                                            \
              InputParameters::Parameter<std::vector<std::vector<std::vector<ptype>>>> *>(par),    \
          in_global,                                                                               \
          global_params_block)

        /**
         * Scalar types
         */
        // built-ins
        // NOTE: Similar dynamic casting is done in InputParameters.C, please update appropriately
        if (false)
          ;
        setscalarvaltype(Real, double, Real);
        setscalarvaltype(int, int, long);
        setscalarvaltype(unsigned short, unsigned int, long);
        setscalarvaltype(long, int, long);
        setscalarvaltype(unsigned int, unsigned int, long);
        setscalarvaltype(unsigned long, unsigned int, long);
        setscalarvaltype(long int, int64_t, long);
        setscalarvaltype(unsigned long long, unsigned int, long);

        setscalar(bool, bool);
        setscalar(SubdomainID, int);
        setscalar(BoundaryID, int);

        // string and string-subclass types
        setscalar(string, string);
        setscalar(SubdomainName, string);
        setscalar(BoundaryName, string);
        setscalar(FileName, string);
        setscalar(MeshFileName, string);
        setscalar(MatrixFileName, string);
        setscalar(FileNameNoExtension, string);
        setscalar(RelativeFileName, string);
        setscalar(DataFileName, string);
        setscalar(ComponentName, string);
        setscalar(PhysicsName, string);
        setscalar(OutFileBase, string);
        setscalar(VariableName, string);
        setscalar(NonlinearVariableName, string);
        setscalar(LinearVariableName, string);
        setscalar(SolverVariableName, string);
        setscalar(AuxVariableName, string);
        setscalar(FunctionName, string);
        setscalar(ConvergenceName, string);
        setscalar(MeshDivisionName, string);
        setscalar(UserObjectName, string);
        setscalar(VectorPostprocessorName, string);
        setscalar(IndicatorName, string);
        setscalar(MarkerName, string);
        setscalar(MultiAppName, string);
        setscalar(OutputName, string);
        setscalar(MaterialPropertyName, string);
        setscalar(MooseFunctorName, string);
        setscalar(MaterialName, string);
        setscalar(DistributionName, string);
        setscalar(PositionsName, string);
        setscalar(SamplerName, string);
        setscalar(TagName, string);
        setscalar(TimesName, string);
        setscalar(MeshGeneratorName, string);
        setscalar(ExtraElementIDName, string);
        setscalar(PostprocessorName, PostprocessorName);
        setscalar(ExecutorName, string);
        setscalar(NonlinearSystemName, string);
        setscalar(LinearSystemName, string);
        setscalar(SolverSystemName, string);
        setscalar(CLIArgString, string);
#ifdef MOOSE_MFEM_ENABLED
        setscalar(MFEMScalarCoefficientName, string);
        setscalar(MFEMVectorCoefficientName, string);
        setscalar(MFEMMatrixCoefficientName, string);
#endif

        // Moose Compound Scalars
        setscalar(RealVectorValue, RealVectorValue);
        setscalar(Point, Point);
        setscalar(RealEigenVector, RealEigenVector);
        setscalar(RealEigenMatrix, RealEigenMatrix);
        setscalar(MooseEnum, MooseEnum);
        setscalar(MultiMooseEnum, MultiMooseEnum);
        setscalar(RealTensorValue, RealTensorValue);
        setscalar(ExecFlagEnum, ExecFlagEnum);
        setscalar(ReporterName, string);
        setscalar(ReporterValueName, string);
        setscalar(ParsedFunctionExpression, string);

        // vector types
        setvector(bool, bool);
        setvector(Real, double);
        setvector(int, int);
        setvector(long, int);
        setvector(unsigned int, int);

// We need to be able to parse 8-byte unsigned types when
// libmesh is configured --with-dof-id-bytes=8.  Officially,
// libmesh uses uint64_t in that scenario, which is usually
// equivalent to 'unsigned long long'.  Note that 'long long'
// has been around since C99 so most C++ compilers support it,
// but presumably uint64_t is the "most standard" way to get a
// 64-bit unsigned type, so we'll stick with that here.
#if LIBMESH_DOF_ID_BYTES == 8
        setvector(uint64_t, int);
#endif

        setvector(SubdomainID, int);
        setvector(BoundaryID, int);
        setvector(RealVectorValue, RealVectorValue);
        setvector(Point, Point);
        setvector(MooseEnum, MooseEnum);
        setvector(MultiMooseEnum, MultiMooseEnum);

        setvector(string, string);
        setvector(FileName, string);
        setvector(FileNameNoExtension, string);
        setvector(RelativeFileName, string);
        setvector(DataFileName, string);
        setvector(MeshFileName, string);
        setvector(MatrixFileName, string);
        setvector(SubdomainName, string);
        setvector(BoundaryName, string);
        setvector(NonlinearVariableName, string);
        setvector(LinearVariableName, string);
        setvector(SolverVariableName, string);
        setvector(AuxVariableName, string);
        setvector(FunctionName, string);
        setvector(ConvergenceName, string);
        setvector(MeshDivisionName, string);
        setvector(UserObjectName, string);
        setvector(IndicatorName, string);
        setvector(MarkerName, string);
        setvector(MultiAppName, string);
        setvector(PostprocessorName, PostprocessorName);
        setvector(VectorPostprocessorName, string);
        setvector(OutputName, string);
        setvector(MaterialPropertyName, string);
        setvector(MooseFunctorName, string);
        setvector(MaterialName, string);
        setvector(DistributionName, string);
        setvector(SamplerName, string);
        setvector(TagName, string);
        setvector(VariableName, VariableName);
        setvector(MeshGeneratorName, string);
        setvector(ExtraElementIDName, string);
        setvector(ReporterName, string);
        setvector(CLIArgString, string);
        setvector(ComponentName, string);
        setvector(PhysicsName, string);
        setvector(PositionsName, string);
        setvector(TimesName, string);
        setvector(ReporterValueName, string);
        setvector(ExecutorName, string);
        setvector(NonlinearSystemName, string);
        setvector(LinearSystemName, string);
        setvector(SolverSystemName, string);
#ifdef MOOSE_MFEM_ENABLED
        setvector(MFEMScalarCoefficientName, string);
        setvector(MFEMVectorCoefficientName, string);
        setvector(MFEMMatrixCoefficientName, string);
#endif

        // map types
        setmap(string, unsigned int);
        setmap(string, Real);
        setmap(string, string);
        setmap(unsigned int, unsigned int);
        setmap(unsigned long, unsigned int);
        setmap(unsigned long long, unsigned int);

        // Double indexed types
        setvectorvector(Real);
        setvectorvector(int);
        setvectorvector(long);
        setvectorvector(unsigned int);
        setvectorvector(unsigned long long);

// See vector type explanation
#if LIBMESH_DOF_ID_BYTES == 8
        setvectorvector(uint64_t);
#endif

        setvectorvector(SubdomainID);
        setvectorvector(BoundaryID);
        setvectorvector(Point);
        setvectorvector(string);
        setvectorvector(FileName);
        setvectorvector(FileNameNoExtension);
        setvectorvector(DataFileName);
        setvectorvector(MeshFileName);
        setvectorvector(MatrixFileName);
        setvectorvector(SubdomainName);
        setvectorvector(BoundaryName);
        setvectorvector(VariableName);
        setvectorvector(NonlinearVariableName);
        setvectorvector(LinearVariableName);
        setvectorvector(SolverVariableName);
        setvectorvector(AuxVariableName);
        setvectorvector(FunctionName);
        setvectorvector(ConvergenceName);
        setvectorvector(UserObjectName);
        setvectorvector(IndicatorName);
        setvectorvector(MarkerName);
        setvectorvector(MultiAppName);
        setvectorvector(PostprocessorName);
        setvectorvector(VectorPostprocessorName);
        setvectorvector(MarkerName);
        setvectorvector(OutputName);
        setvectorvector(MaterialPropertyName);
        setvectorvector(MooseFunctorName);
        setvectorvector(MaterialName);
        setvectorvector(DistributionName);
        setvectorvector(SamplerName);
        setvectorvector(TagName);
#ifdef MOOSE_MFEM_ENABLED
        setvectorvector(MFEMScalarCoefficientName);
        setvectorvector(MFEMVectorCoefficientName);
        setvectorvector(MFEMMatrixCoefficientName);
#endif

        // Triple indexed types
        setvectorvectorvector(Real);
        setvectorvectorvector(int);
        setvectorvectorvector(long);
        setvectorvectorvector(unsigned int);
        setvectorvectorvector(unsigned long long);

// See vector type explanation
#if LIBMESH_DOF_ID_BYTES == 8
        setvectorvectorvector(uint64_t);
#endif

        setvectorvectorvector(SubdomainID);
        setvectorvectorvector(BoundaryID);
        setvectorvectorvector(string);
        setvectorvectorvector(FileName);
        setvectorvectorvector(FileNameNoExtension);
        setvectorvectorvector(DataFileName);
        setvectorvectorvector(MeshFileName);
        setvectorvectorvector(MatrixFileName);
        setvectorvectorvector(SubdomainName);
        setvectorvectorvector(BoundaryName);
        setvectorvectorvector(VariableName);
        setvectorvectorvector(NonlinearVariableName);
        setvectorvectorvector(LinearVariableName);
        setvectorvectorvector(AuxVariableName);
        setvectorvectorvector(FunctionName);
        setvectorvectorvector(UserObjectName);
        setvectorvectorvector(IndicatorName);
        setvectorvectorvector(MarkerName);
        setvectorvectorvector(MultiAppName);
        setvectorvectorvector(PostprocessorName);
        setvectorvectorvector(VectorPostprocessorName);
        setvectorvectorvector(MarkerName);
        setvectorvectorvector(OutputName);
        setvectorvectorvector(MaterialPropertyName);
        setvectorvectorvector(MooseFunctorName);
        setvectorvectorvector(MaterialName);
        setvectorvectorvector(DistributionName);
        setvectorvectorvector(SamplerName);
        else
        {
          _parser.parseError(
              {{"unsupported type '" + par->type() + "' for input parameter '" + full_name + "'",
                node}});
        }

#undef setscalarValueType
#undef setscalar
#undef setvector
#undef setvectorvectorvector
#undef setvectorvector
#undef setmap
        break;
      }
    }

    if (!found)
    {
      /**
       * Special case handling
       *   if the parameter wasn't found in the input file or the cli object the logic in this
       * branch will execute
       */

      // In the case where we have OutFileName but it wasn't actually found in the input filename,
      // we will populate it with the actual parsed filename which is available here in the
      // parser.

      InputParameters::Parameter<OutFileBase> * scalar_p =
          dynamic_cast<InputParameters::Parameter<OutFileBase> *>(MooseUtils::get(it.second));
      if (scalar_p)
      {
        std::string input_file_name = getPrimaryFileName();
        mooseAssert(input_file_name != "", "Input Filename is nullptr");
        size_t pos = input_file_name.find_last_of('.');
        mooseAssert(pos != std::string::npos, "Unable to determine suffix of input file name");
        scalar_p->set() = input_file_name.substr(0, pos) + "_out";
        p.set_attributes(it.first, false);
      }
    }
  }

  // Here we will see if there are any auto build vectors that need to be created
  std::map<std::string, std::pair<std::string, std::string>> auto_build_vectors =
      p.getAutoBuildVectors();
  for (const auto & it : auto_build_vectors)
  {
    // We'll autogenerate values iff the requested vector is not valid but both the base and
    // number
    // are valid
    const std::string & base_name = it.second.first;
    const std::string & num_repeat = it.second.second;

    if (!p.isParamValid(it.first) && p.isParamValid(base_name) && p.isParamValid(num_repeat))
    {
      unsigned int vec_size = p.get<unsigned int>(num_repeat);
      const std::string & name = p.get<std::string>(base_name);

      std::vector<VariableName> variable_names(vec_size);
      for (unsigned int i = 0; i < vec_size; ++i)
      {
        std::ostringstream oss;
        oss << name << i;
        variable_names[i] = oss.str();
      }

      // Finally set the autogenerated vector into the InputParameters object
      p.set<std::vector<VariableName>>(it.first) = variable_names;
    }
  }
}

template <typename T>
bool
toBool(const std::string & /*s*/, T & /*val*/)
{
  return false;
}

template <>
bool
toBool<bool>(const std::string & s, bool & val)
{
  return hit::toBool(s, &val);
}

template <typename T, typename Base>
void
Builder::setScalarParameter(const std::string & full_name,
                            const std::string & short_name,
                            InputParameters::Parameter<T> * param,
                            bool in_global,
                            GlobalParamsAction * global_block)
{
  auto node = _root.find(full_name, true);
  try
  {
    param->set() = node->param<Base>();
  }
  catch (hit::Error & err)
  {
    auto strval = node->param<std::string>();

    auto & t = typeid(T);
    // handle the case where the user put a number inside quotes
    if constexpr (std::is_same_v<T, int> || std::is_same_v<T, unsigned int> ||
                  std::is_same_v<T, SubdomainID> || std::is_same_v<T, BoundaryID> ||
                  std::is_same_v<T, double>)
    {
      try
      {
        param->set() = MooseUtils::convert<T>(strval, true);
      }
      catch (std::invalid_argument & /*e*/)
      {
        const std::string format_type = (t == typeid(double)) ? "float" : "integer";
        _errors.emplace_back("invalid " + format_type + " syntax for parameter: " + full_name +
                                 "='" + strval + "'",
                             node);
      }
    }
    else if constexpr (std::is_same_v<T, bool>)
    {
      if (!toBool(strval, param->set()))
        _errors.emplace_back(
            "invalid boolean syntax for parameter: " + full_name + "='" + strval + "'", node);
    }
    else
      throw;
  }

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<T>(short_name) = param->get();
  }
}

template <typename T, typename UP_T, typename Base>
void
Builder::setScalarValueTypeParameter(const std::string & full_name,
                                     const std::string & short_name,
                                     InputParameters::Parameter<T> * param,
                                     bool in_global,
                                     GlobalParamsAction * global_block,
                                     const hit::Node & node)
{
  setScalarParameter<T, Base>(full_name, short_name, param, in_global, global_block);

  // If this is a range checked param, we need to make sure that the value falls within the
  // requested range
  mooseAssert(_current_params, "Current params is nullptr");

  const auto error = _current_params->rangeCheck<T, UP_T>(full_name, short_name, param);
  if (error)
    _errors.emplace_back(error->second, &node);
}

template <typename T, typename Base>
void
Builder::setVectorParameter(const std::string & full_name,
                            const std::string & short_name,
                            InputParameters::Parameter<std::vector<T>> * param,
                            bool in_global,
                            GlobalParamsAction * global_block)
{
  std::vector<T> vec;
  if (const auto node = _root.find(full_name))
  {
    try
    {
      auto tmp = node->param<std::vector<Base>>();
      for (auto val : tmp)
        vec.push_back(val);
    }
    catch (hit::Error & err)
    {
      Parser::appendErrorMessages(_errors, err);
      return;
    }
  }

  param->set() = vec;

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setVectorParam<T>(short_name).resize(param->get().size());
    for (unsigned int i = 0; i < vec.size(); ++i)
      global_block->setVectorParam<T>(short_name)[i] = param->get()[i];
  }
}

template <typename KeyType, typename MappedType>
void
Builder::setMapParameter(const std::string & full_name,
                         const std::string & short_name,
                         InputParameters::Parameter<std::map<KeyType, MappedType>> * param,
                         bool in_global,
                         GlobalParamsAction * global_block)
{
  std::map<KeyType, MappedType> the_map;

  if (const auto node = _root.find(full_name))
  {
    try
    {
      const auto & string_vec = node->param<std::vector<std::string>>();
      auto it = string_vec.begin();
      while (it != string_vec.end())
      {
        const auto & string_key = *it;
        ++it;
        if (it == string_vec.end())
        {
          _errors.emplace_back(
              "odd number of entries in string vector for map parameter '" + full_name +
                  "'; there must be " +
                  "an even number or else you will end up with a key without a value",
              node);
          return;
        }
        const auto & string_value = *it;
        ++it;

        std::pair<KeyType, MappedType> pr;
        // key
        try
        {
          pr.first = MooseUtils::convert<KeyType>(string_key, true);
        }
        catch (std::invalid_argument & /*e*/)
        {
          _errors.emplace_back("invalid " + MooseUtils::prettyCppType<KeyType>() +
                                   " syntax for map parameter '" + full_name +
                                   "' key: " + string_key,
                               node);
          return;
        }
        // value
        try
        {
          pr.second = MooseUtils::convert<MappedType>(string_value, true);
        }
        catch (std::invalid_argument & /*e*/)
        {
          _errors.emplace_back("invalid " + MooseUtils::prettyCppType<MappedType>() +
                                   " syntax for map parameter '" + full_name +
                                   "' value: " + string_value,
                               node);
          return;
        }

        auto insert_pr = the_map.insert(std::move(pr));
        if (!insert_pr.second)
        {
          _errors.emplace_back("Duplicate map entry for map parameter: '" + full_name + "'; key '" +
                                   string_key + "' appears multiple times",
                               node);
          return;
        }
      }
    }
    catch (hit::Error & err)
    {
      Parser::appendErrorMessages(_errors, err);
      return;
    }
  }

  param->set() = the_map;

  if (in_global)
  {
    global_block->remove(short_name);
    auto & global_map = global_block->setParam<std::map<KeyType, MappedType>>(short_name);
    for (const auto & pair : the_map)
      global_map.insert(pair);
  }
}

template <typename T>
void
Builder::setDoubleIndexParameter(const std::string & full_name,
                                 const std::string & short_name,
                                 InputParameters::Parameter<std::vector<std::vector<T>>> * param,
                                 bool in_global,
                                 GlobalParamsAction * global_block)
{
  auto & value = param->set();

  // Get the full string assigned to the variable full_name
  const auto node = _root.find(full_name, true);
  const auto value_string = MooseUtils::trim(node->param<std::string>());

  // split vector at delim ;
  // NOTE: the substrings are _not_ of type T yet
  // The zero length here is intentional, as we want something like:
  // "abc; 123;" -> ["abc", "123", ""]
  std::vector<std::string> outer_string_vectors;
  // With split, we will get a single entry if the string value is empty. However,
  // that should represent an empty vector<vector>. Therefore, only split if we have values.
  if (!value_string.empty())
    outer_string_vectors = MooseUtils::split(value_string, ";");

  const auto outer_vector_size = outer_string_vectors.size();
  value.resize(outer_vector_size);

  for (const auto j : index_range(outer_string_vectors))
    if (!MooseUtils::tokenizeAndConvert<T>(outer_string_vectors[j], value[j]))
    {
      _errors.emplace_back("invalid format for parameter " + full_name, node);
      return;
    }

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setDoubleIndexParam<T>(short_name).resize(outer_vector_size);
    for (const auto j : make_range(outer_vector_size))
    {
      global_block->setDoubleIndexParam<T>(short_name)[j].resize(value[j].size());
      for (const auto i : index_range(value[j]))
        global_block->setDoubleIndexParam<T>(short_name)[j][i] = value[j][i];
    }
  }
}

template <typename T>
void
Builder::setTripleIndexParameter(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<std::vector<std::vector<std::vector<T>>>> * param,
    bool in_global,
    GlobalParamsAction * global_block)
{
  // Get the full string assigned to the variable full_name
  auto node = _root.find(full_name, true);
  const std::string buffer_raw = node->param<std::string>();
  // In case the parameter is empty
  if (buffer_raw.find_first_not_of(' ', 0) == std::string::npos)
    return;

  // Add a space between neighboring delim's, before the first delim if nothing is ahead of it, and
  // after the last delim if nothing is behind it.
  std::string buffer;
  buffer.push_back(buffer_raw[0]);
  if (buffer[0] == '|' || buffer[0] == ';')
    buffer = ' ' + buffer;
  for (std::string::size_type i = 1; i < buffer_raw.size(); i++)
  {
    if ((buffer_raw[i - 1] == '|' || buffer_raw[i - 1] == ';') &&
        (buffer_raw[i] == '|' || buffer_raw[i] == ';'))
      buffer.push_back(' ');
    buffer.push_back(buffer_raw[i]);
  }
  if (buffer.back() == '|' || buffer.back() == ';')
    buffer.push_back(' ');

  // split vector at delim | to get a series of 2D subvectors
  std::vector<std::string> first_tokenized_vector;
  std::vector<std::vector<std::string>> second_tokenized_vector;
  MooseUtils::tokenize(buffer, first_tokenized_vector, 1, "|");
  param->set().resize(first_tokenized_vector.size());
  second_tokenized_vector.resize(first_tokenized_vector.size());
  for (unsigned j = 0; j < first_tokenized_vector.size(); ++j)
  {
    // Identify empty subvector first
    if (first_tokenized_vector[j].find_first_not_of(' ', 0) == std::string::npos)
    {
      param->set()[j].resize(0);
      continue;
    }
    // split each 2D subvector at delim ; to get 1D sub-subvectors
    // NOTE: the 1D sub-subvectors are _not_ of type T yet
    MooseUtils::tokenize(first_tokenized_vector[j], second_tokenized_vector[j], 1, ";");
    param->set()[j].resize(second_tokenized_vector[j].size());
    for (unsigned k = 0; k < second_tokenized_vector[j].size(); ++k)
      if (!MooseUtils::tokenizeAndConvert<T>(second_tokenized_vector[j][k], param->set()[j][k]))
      {
        _errors.emplace_back("invalid format for '" + full_name + "'", node);
        return;
      }
  }

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setTripleIndexParam<T>(short_name).resize(first_tokenized_vector.size());
    for (unsigned j = 0; j < first_tokenized_vector.size(); ++j)
    {
      global_block->setTripleIndexParam<T>(short_name)[j].resize(second_tokenized_vector[j].size());
      for (unsigned k = 0; k < second_tokenized_vector[j].size(); ++k)
      {
        global_block->setTripleIndexParam<T>(short_name)[j][k].resize(param->get()[j][k].size());
        for (unsigned int i = 0; i < param->get()[j][k].size(); ++i)
          global_block->setTripleIndexParam<T>(short_name)[j][k][i] = param->get()[j][k][i];
      }
    }
  }
}

template <typename T>
void
Builder::setScalarComponentParameter(const std::string & full_name,
                                     const std::string & short_name,
                                     InputParameters::Parameter<T> * param,
                                     bool in_global,
                                     GlobalParamsAction * global_block)
{
  auto node = _root.find(full_name, true);
  std::vector<double> vec;
  try
  {
    vec = node->param<std::vector<double>>();
  }
  catch (hit::Error & err)
  {
    Parser::appendErrorMessages(_errors, err);
    return;
  }

  if (vec.size() != LIBMESH_DIM)
  {
    _errors.emplace_back("wrong number of values in scalar component parameter '" + full_name +
                             "': " + short_name + " was given " + std::to_string(vec.size()) +
                             " components but should have " + std::to_string(LIBMESH_DIM),
                         node);
    return;
  }

  T value;
  for (unsigned int i = 0; i < vec.size(); ++i)
    value(i) = Real(vec[i]);

  param->set() = value;
  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<T>(short_name) = value;
  }
}

template <typename T>
void
Builder::setVectorComponentParameter(const std::string & full_name,
                                     const std::string & short_name,
                                     InputParameters::Parameter<std::vector<T>> * param,
                                     bool in_global,
                                     GlobalParamsAction * global_block)
{
  auto node = _root.find(full_name, true);
  std::vector<double> vec;
  try
  {
    vec = node->param<std::vector<double>>();
  }
  catch (hit::Error & err)
  {
    Parser::appendErrorMessages(_errors, err);
    return;
  }

  if (vec.size() % LIBMESH_DIM)
  {
    _errors.emplace_back("wrong number of values in vector component parameter '" + full_name +
                             "': size " + std::to_string(vec.size()) + " is not a multiple of " +
                             std::to_string(LIBMESH_DIM),
                         node);
    return;
  }

  std::vector<T> values;
  for (unsigned int i = 0; i < vec.size() / LIBMESH_DIM; ++i)
  {
    T value;
    for (int j = 0; j < LIBMESH_DIM; ++j)
      value(j) = Real(vec[i * LIBMESH_DIM + j]);
    values.push_back(value);
  }

  param->set() = values;

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setVectorParam<T>(short_name).resize(vec.size(), values[0]);
    for (unsigned int i = 0; i < vec.size() / LIBMESH_DIM; ++i)
      global_block->setVectorParam<T>(short_name)[i] = values[0];
  }
}

template <typename T>
void
Builder::setVectorVectorComponentParameter(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<std::vector<std::vector<T>>> * param,
    bool in_global,
    GlobalParamsAction * global_block)
{
  // Get the full string assigned to the variable full_name
  auto node = _root.find(full_name, true);
  std::string buffer = node->param<std::string>();

  // split vector at delim ;
  // NOTE: the substrings are _not_ of type T yet
  std::vector<std::string> first_tokenized_vector;
  MooseUtils::tokenize(buffer, first_tokenized_vector, 1, ";");
  param->set().resize(first_tokenized_vector.size());

  // get a vector<vector<double>> first
  std::vector<std::vector<double>> vecvec(first_tokenized_vector.size());
  for (unsigned j = 0; j < vecvec.size(); ++j)
    if (!MooseUtils::tokenizeAndConvert<double>(first_tokenized_vector[j], vecvec[j]))
    {
      _errors.emplace_back("invalid format for parameter " + full_name, node);
      return;
    }

  for (const auto & vec : vecvec)
    if (vec.size() % LIBMESH_DIM)
    {
      _errors.emplace_back("wrong number of values in double-indexed vector component parameter '" +
                               full_name + "': size of subcomponent " + std::to_string(vec.size()) +
                               " is not a multiple of " + std::to_string(LIBMESH_DIM),
                           node);
      return;
    }

  // convert vector<vector<double>> to vector<vector<T>>
  std::vector<std::vector<T>> values(vecvec.size());
  for (unsigned int id_vec = 0; id_vec < vecvec.size(); ++id_vec)
    for (unsigned int i = 0; i < vecvec[id_vec].size() / LIBMESH_DIM; ++i)
    {
      T value;
      for (int j = 0; j < LIBMESH_DIM; ++j)
        value(j) = Real(vecvec[id_vec][i * LIBMESH_DIM + j]);
      values[id_vec].push_back(value);
    }

  param->set() = values;

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setDoubleIndexParam<T>(short_name).resize(vecvec.size());
    for (unsigned j = 0; j < vecvec.size(); ++j)
    {
      global_block->setDoubleIndexParam<T>(short_name)[j].resize(param->get()[j].size() /
                                                                 LIBMESH_DIM);
      for (unsigned int i = 0; i < param->get()[j].size() / LIBMESH_DIM; ++i)
        global_block->setDoubleIndexParam<T>(short_name)[j][i] = values[j][i];
    }
  }
}

template <>
void
Builder::setScalarParameter<RealVectorValue, RealVectorValue>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<RealVectorValue> * param,
    bool in_global,
    GlobalParamsAction * global_block)
{
  setScalarComponentParameter(full_name, short_name, param, in_global, global_block);
}

template <>
void
Builder::setScalarParameter<Point, Point>(const std::string & full_name,
                                          const std::string & short_name,
                                          InputParameters::Parameter<Point> * param,
                                          bool in_global,
                                          GlobalParamsAction * global_block)
{
  setScalarComponentParameter(full_name, short_name, param, in_global, global_block);
}

template <>
void
Builder::setScalarParameter<RealEigenVector, RealEigenVector>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<RealEigenVector> * param,
    bool in_global,
    GlobalParamsAction * global_block)
{
  std::vector<double> vec;
  try
  {
    vec = _root.param<std::vector<double>>(full_name);
  }
  catch (hit::Error & err)
  {
    Parser::appendErrorMessages(_errors, err);
    return;
  }

  RealEigenVector value(vec.size());
  for (unsigned int i = 0; i < vec.size(); ++i)
    value(i) = Real(vec[i]);

  param->set() = value;
  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<RealEigenVector>(short_name) = value;
  }
}

template <>
void
Builder::setScalarParameter<RealEigenMatrix, RealEigenMatrix>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<RealEigenMatrix> * param,
    bool in_global,
    GlobalParamsAction * global_block)
{
  // Get the full string assigned to the variable full_name
  auto node = _root.find(full_name, true);
  std::string buffer = node->param<std::string>();

  // split vector at delim ;
  // NOTE: the substrings are _not_ of type T yet
  std::vector<std::string> first_tokenized_vector;
  MooseUtils::tokenize(buffer, first_tokenized_vector, 1, ";");

  std::vector<std::vector<Real>> values(first_tokenized_vector.size());

  for (unsigned j = 0; j < first_tokenized_vector.size(); ++j)
  {
    if (!MooseUtils::tokenizeAndConvert<Real>(first_tokenized_vector[j], values[j]) ||
        (j != 0 && values[j].size() != values[0].size()))
    {
      _errors.emplace_back("invalid format for parameter " + full_name, node);
      return;
    }
  }

  RealEigenMatrix value(values.size(), values[0].size());
  for (unsigned int i = 0; i < values.size(); ++i)
    for (unsigned int j = 0; j < values[i].size(); ++j)
      value(i, j) = values[i][j];

  param->set() = value;
  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<RealEigenMatrix>(short_name) = value;
  }
}

template <>
void
Builder::setScalarParameter<MooseEnum, MooseEnum>(const std::string & full_name,
                                                  const std::string & short_name,
                                                  InputParameters::Parameter<MooseEnum> * param,
                                                  bool in_global,
                                                  GlobalParamsAction * global_block)
{
  MooseEnum current_param = param->get();

  std::string value = _root.param<std::string>(full_name);

  param->set() = value;
  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<MooseEnum>(short_name) = current_param;
  }
}

template <>
void
Builder::setScalarParameter<MultiMooseEnum, MultiMooseEnum>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<MultiMooseEnum> * param,
    bool in_global,
    GlobalParamsAction * global_block)
{
  MultiMooseEnum current_param = param->get();

  auto vec = _root.param<std::vector<std::string>>(full_name);

  std::string raw_values;
  for (unsigned int i = 0; i < vec.size(); ++i)
    raw_values += ' ' + vec[i];

  param->set() = raw_values;

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<MultiMooseEnum>(short_name) = current_param;
  }
}

template <>
void
Builder::setScalarParameter<ExecFlagEnum, ExecFlagEnum>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<ExecFlagEnum> * param,
    bool in_global,
    GlobalParamsAction * global_block)
{
  ExecFlagEnum current_param = param->get();
  auto vec = _root.param<std::vector<std::string>>(full_name);

  std::string raw_values;
  for (unsigned int i = 0; i < vec.size(); ++i)
    raw_values += ' ' + vec[i];

  param->set() = raw_values;

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<ExecFlagEnum>(short_name) = current_param;
  }
}

template <>
void
Builder::setScalarParameter<RealTensorValue, RealTensorValue>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<RealTensorValue> * param,
    bool in_global,
    GlobalParamsAction * global_block)
{
  auto node = _root.find(full_name, true);
  auto vec = node->param<std::vector<double>>();
  if (vec.size() != LIBMESH_DIM * LIBMESH_DIM)
  {
    _errors.emplace_back("invalid RealTensorValue parameter '" + full_name + "': size is " +
                             std::to_string(vec.size()) + " but should be " +
                             std::to_string(LIBMESH_DIM * LIBMESH_DIM),
                         node);
    return;
  }

  RealTensorValue value;
  for (int i = 0; i < LIBMESH_DIM; ++i)
    for (int j = 0; j < LIBMESH_DIM; ++j)
      value(i, j) = Real(vec[i * LIBMESH_DIM + j]);

  param->set() = value;
  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<RealTensorValue>(short_name) = value;
  }
}

// Specialization for coupling a Real value where a postprocessor would be needed in MOOSE
template <>
void
Builder::setScalarParameter<PostprocessorName, PostprocessorName>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<PostprocessorName> * param,
    bool in_global,
    GlobalParamsAction * global_block)
{
  PostprocessorName pps_name = _root.param<std::string>(full_name);
  param->set() = pps_name;

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<PostprocessorName>(short_name) = pps_name;
  }
}

template <>
void
Builder::setScalarParameter<ReporterName, std::string>(
    const std::string & full_name,
    const std::string & /*short_name*/,
    InputParameters::Parameter<ReporterName> * param,
    bool /*in_global*/,
    GlobalParamsAction * /*global_block*/)
{
  auto node = _root.find(full_name, true);
  std::vector<std::string> names = MooseUtils::rsplit(node->param<std::string>(), "/", 2);
  if (names.size() != 2)
    _errors.emplace_back(
        "The supplied name ReporterName '" + full_name + "' must contain the '/' delimiter.", node);
  else
    param->set() = ReporterName(names[0], names[1]);
}

template <>
void
Builder::setVectorParameter<RealVectorValue, RealVectorValue>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<std::vector<RealVectorValue>> * param,
    bool in_global,
    GlobalParamsAction * global_block)
{
  setVectorComponentParameter(full_name, short_name, param, in_global, global_block);
}

template <>
void
Builder::setVectorParameter<Point, Point>(const std::string & full_name,
                                          const std::string & short_name,
                                          InputParameters::Parameter<std::vector<Point>> * param,
                                          bool in_global,
                                          GlobalParamsAction * global_block)
{
  setVectorComponentParameter(full_name, short_name, param, in_global, global_block);
}

template <>
void
Builder::setVectorParameter<MooseEnum, MooseEnum>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<std::vector<MooseEnum>> * param,
    bool in_global,
    GlobalParamsAction * global_block)
{
  std::vector<MooseEnum> enum_values = param->get();
  std::vector<std::string> values(enum_values.size());
  for (unsigned int i = 0; i < values.size(); ++i)
    values[i] = static_cast<std::string>(enum_values[i]);

  /**
   * With MOOSE Enums we need a default object so it should have been passed in the param pointer.
   * We are only going to use the first item in the vector (values[0]) and ignore the rest.
   */
  std::vector<std::string> vec;
  if (auto node = _root.find(full_name, true))
  {
    vec = node->param<std::vector<std::string>>();
    param->set().resize(vec.size(), enum_values[0]);
  }

  for (unsigned int i = 0; i < vec.size(); ++i)
    param->set()[i] = vec[i];

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setVectorParam<MooseEnum>(short_name).resize(vec.size(), enum_values[0]);
    for (unsigned int i = 0; i < vec.size(); ++i)
      global_block->setVectorParam<MooseEnum>(short_name)[i] = values[0];
  }
}

template <>
void
Builder::setVectorParameter<MultiMooseEnum, MultiMooseEnum>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<std::vector<MultiMooseEnum>> * param,
    bool in_global,
    GlobalParamsAction * global_block)
{
  const auto node = _root.find(full_name, true);
  const std::vector<MultiMooseEnum> & enum_values = param->get();

  // Get the full string assigned to the variable full_name
  std::string buffer = node->param<std::string>();

  bool has_empty = false;
  const auto first_tokenized_vector = MooseUtils::split(buffer, ";");
  for (const auto i : index_range(first_tokenized_vector))
  {
    const auto & entry = first_tokenized_vector[i];
    if (MooseUtils::trim(entry) == "")
    {
      has_empty = true;
      _errors.emplace_back("entry " + std::to_string(i) + " in '" + node->fullpath() + "' is empty",
                           node);
    }
  }

  if (has_empty)
    return;

  param->set().resize(first_tokenized_vector.size(), enum_values[0]);

  std::vector<std::vector<std::string>> vecvec(first_tokenized_vector.size());
  for (const auto i : index_range(vecvec))
  {
    MooseUtils::tokenize<std::string>(first_tokenized_vector[i], vecvec[i], 1, " ");
    param->set()[i] = vecvec[i];
  }

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setVectorParam<MultiMooseEnum>(short_name).resize(vecvec.size(), enum_values[0]);
    for (unsigned int i = 0; i < vecvec.size(); ++i)
      global_block->setVectorParam<MultiMooseEnum>(short_name)[i] = vecvec[i];
  }
}

template <>
void
Builder::setVectorParameter<PostprocessorName, PostprocessorName>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<std::vector<PostprocessorName>> * param,
    bool in_global,
    GlobalParamsAction * global_block)
{
  std::vector<std::string> pps_names = _root.param<std::vector<std::string>>(full_name);
  unsigned int n = pps_names.size();
  param->set().resize(n);

  for (unsigned int j = 0; j < n; ++j)
    param->set()[j] = pps_names[j];

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setVectorParam<PostprocessorName>(short_name).resize(n, "");
    for (unsigned int j = 0; j < n; ++j)
      global_block->setVectorParam<PostprocessorName>(short_name)[j] = pps_names[j];
  }
}

/**
 * Specialization for coupling vectors. This routine handles default values and auto generated
 * VariableValue vectors.
 */
template <>
void
Builder::setVectorParameter<VariableName, VariableName>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<std::vector<VariableName>> * param,
    bool /*in_global*/,
    GlobalParamsAction * /*global_block*/)
{
  auto node = _root.find(full_name, true);
  auto vec = node->param<std::vector<std::string>>();
  auto strval = node->param<std::string>();
  std::vector<VariableName> var_names(vec.size());

  bool has_var_names = false;
  for (unsigned int i = 0; i < vec.size(); ++i)
  {
    VariableName var_name = vec[i];

    Real real_value;
    std::istringstream ss(var_name);

    // If we are able to convert this value into a Real, then set a default coupled value
    // NOTE: parameter must be either all default or no defaults
    if (ss >> real_value && ss.eof())
      _current_params->defaultCoupledValue(short_name, real_value, i);
    else
    {
      var_names[i] = var_name;
      has_var_names = true;
    }
  }

  if (has_var_names)
  {
    param->set().resize(vec.size());

    for (unsigned int i = 0; i < vec.size(); ++i)
      if (var_names[i] == "")
        _errors.emplace_back("invalid value for '" + full_name +
                                 "': coupled vectors where some parameters are reals and others "
                                 "are variables are not supported",
                             node);
      else
        param->set()[i] = var_names[i];
  }
}

template <>
void
Builder::setVectorParameter<ReporterName, std::string>(
    const std::string & full_name,
    const std::string & /*short_name*/,
    InputParameters::Parameter<std::vector<ReporterName>> * param,
    bool /*in_global*/,
    GlobalParamsAction * /*global_block*/)
{
  auto node = _root.find(full_name, true);
  auto rnames = node->param<std::vector<std::string>>();
  param->set().resize(rnames.size());

  for (unsigned int i = 0; i < rnames.size(); ++i)
  {
    std::vector<std::string> names = MooseUtils::rsplit(rnames[i], "/", 2);
    if (names.size() != 2)
      _errors.emplace_back("the supplied name ReporterName '" + rnames[i] +
                               "' must contain the '/' delimiter",
                           node);
    else
      param->set()[i] = ReporterName(names[0], names[1]);
  }
}

template <>
void
Builder::setVectorParameter<CLIArgString, std::string>(
    const std::string & full_name,
    const std::string & /*short_name*/,
    InputParameters::Parameter<std::vector<CLIArgString>> * param,
    bool /*in_global*/,
    GlobalParamsAction * /*global_block*/)
{
  // Parsed as a vector of string, the vectors parameters are being cut
  auto rnames = _root.param<std::vector<std::string>>(full_name);
  param->set().resize(rnames.size()); // slightly oversized if vectors have been split

  // Skip empty parameter
  if (rnames.empty())
    return;

  // Re-assemble vector parameters
  unsigned int i_param = 0;
  bool vector_param_detected = false;
  for (unsigned int i = 0; i < rnames.size(); ++i)
  {
    // Look for a quote, both types
    std::vector<std::string> double_split =
        MooseUtils::rsplit(rnames[i], "\"", std::numeric_limits<std::size_t>::max());
    std::vector<std::string> single_split =
        MooseUtils::rsplit(rnames[i], "\'", std::numeric_limits<std::size_t>::max());
    if (double_split.size() + single_split.size() >= 3)
      // Either entering or exiting a vector parameter (>3 is entering another vector)
      // Even and >2 number of quotes means both finished and started another vector parameter
      if ((double_split.size() + single_split.size()) % 2 == 1)
        vector_param_detected = !vector_param_detected;

    // We're building a vector parameters, just append the text, rebuild the spaces
    if (vector_param_detected)
      param->set()[i_param] += rnames[i] + ' ';
    else
    {
      param->set()[i_param] += rnames[i];
      i_param++;
    }
  }
  // Use actual size after re-forming vector parameters
  param->set().resize(i_param);
}

template <>
void
Builder::setDoubleIndexParameter<Point>(
    const std::string & full_name,
    const std::string & short_name,
    InputParameters::Parameter<std::vector<std::vector<Point>>> * param,
    bool in_global,
    GlobalParamsAction * global_block)
{
  setVectorVectorComponentParameter(full_name, short_name, param, in_global, global_block);
}
} // end of namespace Moose
