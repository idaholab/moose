//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
#include "MooseObjectAction.h"
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
#include "AppFactory.h"

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

Builder::Builder(MooseApp & app,
                 ActionWarehouse & action_wh,
                 std::shared_ptr<Parser> parser,
                 const AppBuilder::State & app_builder_state)
  : BuilderBase(parser),
    ConsoleStreamInterface(app),
    _app(app),
    _factory(app.getFactory()),
    _action_wh(action_wh),
    _action_factory(app.getActionFactory()),
    _syntax(_action_wh.syntax()),
    _app_builder_state(app_builder_state),
    _syntax_formatter(nullptr),
    _sections_read(false)
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
Builder::listValidParams(const std::string & section_name)
{
  std::vector<std::string> paramlist;

  if (section_name == "Application")
  {
    const auto app_params = AppFactory::instance().getValidParams(_app.type());
    for (const auto & it : app_params)
      paramlist.push_back(it.first);
  }
  else
  {
    bool dummy;
    std::string registered_identifier = _syntax.isAssociated(section_name, &dummy);
    auto iters = _syntax.getActions(registered_identifier);

    for (auto it = iters.first; it != iters.second; ++it)
    {
      auto params = _action_factory.getValidParams(it->second._action);
      for (const auto & it : params)
        paramlist.push_back(it.first);
    }
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
      errors.push_back(hit::errormsg(
          n, "unused parameter '", fullpath, "'\n", "      Did you mean '", candidates[0], "'?"));
    else
      errors.push_back(hit::errormsg(n, "unused parameter '", fullpath, "'"));
  }
}

void
Builder::walkRaw(std::string /*fullpath*/, std::string /*nodepath*/, hit::Node * n)
{
  InputParameters active_list_params = Action::validParams();
  InputParameters params = EmptyAction::validParams();

  std::string section_name = n->fullpath();
  std::string curr_identifier = n->fullpath();

  // Before we retrieve any actions or build any objects, make sure that the section they are in
  // is active
  if (!isSectionActive(curr_identifier, &root()))
    return;
  // The AppBuilder manages the [Application] block
  if (curr_identifier == "Application")
    return;

  // Extract the block parameters before constructing the action
  // There may be more than one Action registered for a given section in which case we need to
  // build them all
  bool is_parent;
  std::string registered_identifier = _syntax.isAssociated(section_name, &is_parent);

  // Make sure at least one action is associated with the current identifier
  if (const auto [begin, end] = _syntax.getActions(registered_identifier); begin == end)
  {
    _errmsg += hit::errormsg(n,
                             "section '[",
                             curr_identifier,
                             "]' does not have an associated \"Action\".\n Common causes:\n"
                             "- you misspelled the Action/section name\n"
                             "- the app you are running does not support this Action/syntax") +
               "\n";
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

    extractParams(curr_identifier, params);

    // Add the parsed syntax to the parameters object for consumption by the Action
    params.set<std::string>("task") = it->second._task;
    params.set<std::string>("registered_identifier") = registered_identifier;
    params.blockLocation() = n->filename() + ":" + std::to_string(n->line());
    params.blockFullpath() = n->fullpath();

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
        object_action->getObjectParams().blockLocation() = params.blockLocation();
        object_action->getObjectParams().blockFullpath() = params.blockFullpath();
        extractParams(curr_identifier, object_action->getObjectParams());
        object_action->getObjectParams()
            .set<std::vector<std::string>>("control_tags")
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
  // At this point, the AppBuilder has already merged in the CLI arguments
  // and has done the initial walk for expansion, duplicate paremeter checks,
  // and active/inactive checks

  // Get the variables that we have already extracted from [Application] and the
  // fparser expansion fields
  _extracted_vars = _app_builder_state.extracted_vars;

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

  // walk all the sections extracting paramters from each into InputParameters objects
  for (auto & sec : _secs_need_first)
  {
    auto n = root().find(sec);
    if (n)
      walkRaw(n->parent()->fullpath(), n->path(), n);
  }
  root().walk(this, hit::NodeType::Section);

  if (_errmsg.size() > 0)
    mooseError(_errmsg);
}

// Checks the input and the way it has been used and emits any errors/warnings.
// This has to be a separate function because for we don't know if some parameters were unused
// until all the multiapps/subapps have been fully initialized - which isn't complete until
// *after* all the other member functions on Parser have been run.  So this is here to be
// externally called at the right time.
void
Builder::errorCheck(const Parallel::Communicator & comm, bool warn_unused, bool err_unused)
{
  UnusedWalker uw(_extracted_vars, *this);
  UnusedWalker uwcli(_extracted_vars, *this);

  root().walk(&uw);
  _app_builder_state.cli_root->walk(&uwcli);

  auto cli = _app.commandLine();
  if (warn_unused)
  {
    for (const auto & arg : _app.commandLine()->unusedHitParams(comm))
      _warnmsg +=
          hit::errormsg("CLI_ARG", nullptr, "unused command line parameter '", arg, "'") + "\n";
    for (auto & msg : uwcli.errors)
      _warnmsg += msg + "\n";
    for (auto & msg : uw.errors)
      _warnmsg += msg + "\n";
  }
  else if (err_unused)
  {
    for (const auto & arg : cli->unusedHitParams(comm))
      _errmsg +=
          hit::errormsg("CLI_ARG", nullptr, "unused command line parameter '", arg, "'") + "\n";
    for (auto & msg : uwcli.errors)
      _errmsg += msg + "\n";
    for (auto & msg : uw.errors)
      _errmsg += msg + "\n";
  }

  if (_warnmsg.size() > 0)
    mooseUnused(_warnmsg);
  if (_errmsg.size() > 0)
    mooseError(_errmsg);
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

  for (const auto & act_names : all_names)
  {
    const auto & act_info = act_names.second;
    const std::string & action = act_info._action;
    const std::string & task = act_info._task;
    const std::string act_name = act_names.first;
    InputParameters action_obj_params = _action_factory.getValidParams(action);
    bool params_added = root.addParameters("",
                                           act_name,
                                           false,
                                           action,
                                           true,
                                           &action_obj_params,
                                           _syntax.getLineInfo(act_name, action, ""),
                                           "");

    if (params_added)
    {
      auto tasks = _action_factory.getTasksByAction(action);
      for (auto & t : tasks)
      {
        auto info = _action_factory.getLineInfo(action, t);
        root.addActionTask(act_name, action, t, info);
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
                 buildable_types.end()) &&                                 // Restricted but found
            moose_obj_params.have_parameter<std::string>("_moose_base") && // Has a registered base
            _syntax.verifyMooseObjectTask(moose_obj_params.get<std::string>("_moose_base"),
                                          task) &&          // and that base is associated
            action_obj_params.mooseObjectSyntaxVisibility() // and the Action says it's visible
        )
        {
          std::string name;
          size_t pos = 0;
          bool is_action_params = false;
          bool is_type = false;
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
            is_type = true;
          }
          moose_obj_params.set<std::string>("type") = moose_obj_name;

          auto lineinfo = _factory.getLineInfo(moose_obj_name);
          std::string classname = _factory.associatedClassName(moose_obj_name);
          root.addParameters(act_name,
                             name,
                             is_type,
                             moose_obj_name,
                             is_action_params,
                             &moose_obj_params,
                             lineinfo,
                             classname);
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
                 buildable_types.end()) &&                                 // Restricted but found
            moose_obj_params.have_parameter<std::string>("_moose_base") && // Has a registered base
            _syntax.verifyMooseObjectTask(moose_obj_params.get<std::string>("_moose_base"),
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

void
Builder::extractParams(const std::string & prefix, InputParameters & p)
{
  static const std::string global_params_task = "set_global_params";
  static const std::string global_params_block_name =
      _syntax.getSyntaxByAction("GlobalParamsAction").front();

  ActionIterator act_iter = _action_wh.actionBlocksWithActionBegin(global_params_task);
  GlobalParamsAction * global_params_block = nullptr;

  // We are grabbing only the first
  if (act_iter != _action_wh.actionBlocksWithActionEnd(global_params_task))
    global_params_block = dynamic_cast<GlobalParamsAction *>(*act_iter);

  BuilderBase::extractParams(prefix, p, global_params_block, global_params_block_name);
}

} // end of namespace Moose
