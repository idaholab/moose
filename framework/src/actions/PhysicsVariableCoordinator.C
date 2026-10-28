//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhysicsVariableCoordinator.h"
#include "ActionWarehouse.h"
#include "ActionFactory.h"
#include "FEProblemBase.h"
#include "MooseMesh.h"

registerMooseAction("MooseApp", PhysicsVariableCoordinator, "flush_shared_physics_variables");

namespace
{
/// Printed value of each non-private, valid parameter other than 'block'. Values are compared in
/// printed form because parameter values have no type-erased equality comparison.
std::map<std::string, std::string>
comparableValues(const InputParameters & params)
{
  std::map<std::string, std::string> values;
  for (const auto & [name, value] : params)
    if (name != "block" && !params.isPrivate(name) && params.isParamValid(name))
    {
      std::stringstream ss;
      value->print(ss);
      values.emplace(name, ss.str());
    }
  return values;
}

/// Names of the parameters whose comparable values (see comparableValues()) differ
std::vector<std::string>
differingParameters(const InputParameters & a, const InputParameters & b)
{
  const auto a_values = comparableValues(a);
  const auto b_values = comparableValues(b);
  std::set<std::string> names;
  for (const auto & [name, _] : a_values)
    names.insert(name);
  for (const auto & [name, _] : b_values)
    names.insert(name);

  std::vector<std::string> differing;
  for (const auto & name : names)
  {
    const auto a_it = a_values.find(name);
    const auto b_it = b_values.find(name);
    if (a_it == a_values.end() || b_it == b_values.end() || a_it->second != b_it->second)
      differing.push_back(name);
  }
  return differing;
}
}

InputParameters
PhysicsVariableCoordinator::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Merges the requests of multiple Physics for the same shared variable into a single "
      "variable spanning the union of their blocks. Created on demand; not meant to be declared "
      "in an input file.");
  return params;
}

PhysicsVariableCoordinator::PhysicsVariableCoordinator(const InputParameters & params)
  : Action(params)
{
}

void
PhysicsVariableCoordinator::requestVariable(const bool nl,
                                            const std::string & var_type,
                                            const VariableName & var_name,
                                            const InputParameters & params,
                                            const std::vector<SubdomainName> & blocks,
                                            const std::string & requester)
{
  const auto it = _requests.find(var_name);
  if (it == _requests.end())
  {
    _requests.emplace(var_name, MergedRequest{nl, var_type, params, {{requester, blocks}}});
    return;
  }

  auto & merged = it->second;
  const auto & first_requester = merged._requester_blocks.front().first;
  if (merged._nl != nl)
    mooseError("Physics '",
               requester,
               "' requests the shared variable '",
               var_name,
               "' as ",
               nl ? "a solver" : "an auxiliary",
               " variable, but Physics '",
               first_requester,
               "' requested it as ",
               merged._nl ? "a solver" : "an auxiliary",
               " variable.");
  if (merged._type != var_type)
    mooseError("Physics '",
               requester,
               "' requests the shared variable '",
               var_name,
               "' with type '",
               var_type,
               "', but Physics '",
               first_requester,
               "' requested it with type '",
               merged._type,
               "'.");
  const auto differing = differingParameters(params, merged._params);
  if (!differing.empty())
    mooseError("Physics '",
               requester,
               "' requests the shared variable '",
               var_name,
               "' with different values for parameter(s) ",
               Moose::stringify(differing),
               " than Physics '",
               first_requester,
               "' did.");

  merged._requester_blocks.emplace_back(requester, blocks);
}

void
PhysicsVariableCoordinator::act()
{
  for (const auto & [var_name, req] : _requests)
  {
    // ANY_BLOCK_ID cannot be passed to addVariable(); leaving 'block' unset has the same meaning
    std::vector<SubdomainName> blocks;
    bool any_block = false;
    for (const auto & [_, requested_blocks] : req._requester_blocks)
      for (const auto & block : requested_blocks)
        if (block == "ANY_BLOCK_ID")
          any_block = true;
        else if (std::find(blocks.begin(), blocks.end(), block) == blocks.end())
          blocks.push_back(block);

    InputParameters params = req._params;
    if (!any_block)
      params.set<std::vector<SubdomainName>>("block") = blocks;
    // As done by AddVariableAction for variables declared in the input
    if (params.get<bool>("fv"))
      _problem->needFV();
    if (req._nl)
      _problem->addVariable(req._type, var_name, params);
    else
      _problem->addAuxVariable(req._type, var_name, params);

    // After adding the variable so that invalid block names are reported by the variable
    checkFVAdjacency(var_name, req);
  }
}

void
PhysicsVariableCoordinator::checkFVAdjacency(const VariableName & var_name,
                                             const MergedRequest & req) const
{
  if (!req._params.get<bool>("fv") || req._requester_blocks.size() < 2)
    return;

  // Indices of the requesters covering each subdomain, with ANY_BLOCK_ID covering every subdomain
  std::map<SubdomainID, std::set<std::size_t>> sub_to_requesters;
  for (const auto i : index_range(req._requester_blocks))
  {
    const auto & blocks = req._requester_blocks[i].second;
    const std::set<SubdomainID> ids =
        std::find(blocks.begin(), blocks.end(), "ANY_BLOCK_ID") != blocks.end()
            ? _mesh->meshSubdomains()
            : _mesh->getSubdomainIDs(std::set<SubdomainName>(blocks.begin(), blocks.end()));
    for (const auto id : ids)
      sub_to_requesters[id].insert(i);
  }

  const auto sub_name = [this](const SubdomainID id)
  {
    const auto & name = _mesh->getSubdomainName(id);
    return name.empty() ? std::to_string(id) : name;
  };

  // Every face between two of the variable's subdomains must be interior to some requester;
  // otherwise each requester's flux kernels treat that face as a boundary of their own blocks
  for (const auto & [sub_a, requesters_a] : sub_to_requesters)
    for (const auto sub_b : _mesh->getBlockConnectedBlocks(sub_a))
    {
      const auto it_b = sub_to_requesters.find(sub_b);
      // Each pair of subdomains is only examined once
      if (sub_b < sub_a || it_b == sub_to_requesters.end())
        continue;
      const auto & requesters_b = it_b->second;

      const bool some_requester_covers_both =
          std::any_of(requesters_a.begin(),
                      requesters_a.end(),
                      [&requesters_b](const auto i) { return requesters_b.count(i); });
      if (!some_requester_covers_both)
        mooseError("Physics '",
                   req._requester_blocks[*requesters_a.begin()].first,
                   "' and '",
                   req._requester_blocks[*requesters_b.begin()].first,
                   "' share the finite volume variable '",
                   var_name,
                   "' on the adjacent subdomains '",
                   sub_name(sub_a),
                   "' and '",
                   sub_name(sub_b),
                   "', but neither is defined on both. Their flux kernels would treat the faces "
                   "between these subdomains as boundaries, so no flux would cross them. Define a "
                   "single Physics on both subdomains instead.");
    }
}

PhysicsVariableCoordinator &
PhysicsVariableCoordinator::findOrCreate(ActionWarehouse & awh, ActionFactory & action_factory)
{
  const auto existing = awh.getActions<PhysicsVariableCoordinator>();
  if (!existing.empty())
    return *const_cast<PhysicsVariableCoordinator *>(existing[0]);

  const std::string class_name = "PhysicsVariableCoordinator";
  InputParameters params = action_factory.getValidParams(class_name);
  params.set<bool>("_built_by_moose") = true;
  params.set<std::string>("registered_identifier") = "(AutoBuilt)";

  auto action = action_factory.create(class_name, "physics_variable_coordinator", params);
  auto * coordinator = dynamic_cast<PhysicsVariableCoordinator *>(action.get());
  if (!coordinator)
    ::mooseError("Internal error: failed to create the PhysicsVariableCoordinator");
  awh.addActionBlock(action);
  return *coordinator;
}
