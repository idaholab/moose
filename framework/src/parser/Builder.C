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
#include "Parser.h"
#include "MooseObjectAction.h"
#include "AddActionComponentAction.h"
#include "ActionWarehouse.h"
#include "EmptyAction.h"
#include "MooseApp.h"
#include "GlobalParamsAction.h"
#include "SyntaxTree.h"
#include "InputFileFormatter.h"
#include "MooseTypes.h"
#include "CommandLine.h"
#include "Syntax.h"
#include "ParameterExtraction.h"
#include "ParseUtils.h"

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
    _action_wh(action_wh),
    _action_factory(app.getActionFactory()),
    _syntax(_action_wh.syntax()),
    _parser(parser),
    _root(_parser.getRoot())
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

  // Pull in the extraction info from the [Application] block, which was extracted first
  appendExtractionInfo(_app.getAppExtractionInfo());

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

  // Skip the [Application] block as we've already extracted it
  _secs_need_first.push_back("Application");

  // Walk for the remainder
  _root.walk(this, hit::NodeType::Section);

  // Warn for all deprecated parameters together
  if (_deprecated_params.size())
  {
    std::vector<std::string> messages;
    for (const auto & key_message_pair : _deprecated_params)
      messages.push_back(key_message_pair.second);
    const auto message = MooseUtils::stringJoin(messages, "\n\n");

    Moose::ScopedHideTrace scoped_hide_trace;
    moose::internal::mooseDeprecatedStream(Moose::out, false, true, message + "\n\n");
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
    Moose::ParseUtils::appendErrorMessages(messages, uw.errors);
  }

  {
    UnusedWalker uw(_extracted_vars, *this);
    _root.walk(&uw);
    Moose::ParseUtils::appendErrorMessages(messages, uw.errors);
  }

  for (const auto & arg : _app.commandLine()->unusedHitParams(comm))
    messages.emplace_back("unused command line parameter '" + arg + "'");

  if (messages.size())
  {
    const auto message = Moose::ParseUtils::joinErrorMessages(messages);
    if (warn_unused)
      mooseUnused(message);
    if (err_unused)
    {
      if (_parser.getThrowOnError())
        Moose::ParseUtils::parseError(messages, true);
      else
        mooseError(
            message +
            "\n\nAppend --allow-unused (or -w) on the command line to ignore unused parameters.");
    }
  }
}

void
Builder::extractParams(const hit::Node * const section_node, InputParameters & p)
{
  const auto info =
      ParameterExtraction::extract(_root, &_parser.getCommandLineRoot(), section_node, p);
  appendExtractionInfo(info);
}

void
Builder::extractParams(const std::string & prefix, InputParameters & p)
{
  const auto info = ParameterExtraction::extract(_root, &_parser.getCommandLineRoot(), prefix, p);
  appendExtractionInfo(info);
}

void
Builder::appendExtractionInfo(const ParameterExtraction::ExtractionInfo & info)
{
  Moose::ParseUtils::appendErrorMessages(_errors, info.errors);

  // Append to the variables we have extracted, which also
  // includes the paths that have errors (so that we don't
  // report both an error and an unused variable)
  for (const auto & error : info.errors)
  {
    mooseAssert(error.node, "Should have a node");
    _extracted_vars.insert(error.node->fullpath());
  }
  for (const auto node_ptr : info.extracted_nodes)
    _extracted_vars.insert(node_ptr->fullpath());

  for (const auto & [k, v] : info.deprecated_params)
    _deprecated_params.emplace(k, v);
}

} // end of namespace Moose
