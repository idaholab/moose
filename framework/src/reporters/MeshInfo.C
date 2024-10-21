//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshInfo.h"
#include "SubProblem.h"
#include "libmesh/system.h"
#include "libmesh/equation_systems.h"
#include "libmesh/parallel_sync.h"

registerMooseObject("MooseApp", MeshInfo);

InputParameters
MeshInfo::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription(
      "Report mesh information, such as the number of elements, nodes, and degrees of freedom.");

  MultiMooseEnum items(
      "num_dofs num_dofs_nonlinear num_dofs_auxiliary num_elements num_nodes num_local_dofs "
      "num_local_dofs_nonlinear num_local_dofs_auxiliary num_local_elements num_local_nodes "
      "local_sidesets local_sideset_elems sidesets sideset_elems local_subdomains "
      "local_subdomain_elems subdomains subdomain_elems");
  params.addParam<MultiMooseEnum>(
      "items",
      items,
      "The iteration information to output, if nothing is provided everything will be output.");

  return params;
}

MeshInfo::MeshInfo(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _items(getParam<MultiMooseEnum>("items")),
    _num_dofs(declareHelper<unsigned int>("num_dofs", REPORTER_MODE_REPLICATED)),
    _num_dofs_nl(declareHelper<unsigned int>("num_dofs_nonlinear", REPORTER_MODE_REPLICATED)),
    _num_dofs_aux(declareHelper<unsigned int>("num_dofs_auxiliary", REPORTER_MODE_REPLICATED)),
    _num_dofs_constrained(
        declareHelper<unsigned int>("num_dofs_constrained", REPORTER_MODE_REPLICATED)),
    _num_elem(declareHelper<unsigned int>("num_elements", REPORTER_MODE_REPLICATED)),
    _num_node(declareHelper<unsigned int>("num_nodes", REPORTER_MODE_REPLICATED)),
    _num_local_dofs(declareHelper<unsigned int>("num_local_dofs", REPORTER_MODE_DISTRIBUTED)),
    _num_local_dofs_nl(
        declareHelper<unsigned int>("num_dofs_local_nonlinear", REPORTER_MODE_DISTRIBUTED)),
    _num_local_dofs_aux(
        declareHelper<unsigned int>("num_dofs_local_auxiliary", REPORTER_MODE_DISTRIBUTED)),
    _num_local_elem(declareHelper<unsigned int>("num_local_elements", REPORTER_MODE_DISTRIBUTED)),
    _num_local_node(declareHelper<unsigned int>("num_local_nodes", REPORTER_MODE_DISTRIBUTED)),

    _local_sidesets(declareHelper<std::map<BoundaryID, SidesetInfo>>("local_sidesets",
                                                                     REPORTER_MODE_DISTRIBUTED)),
    _local_sideset_elems(declareHelper<std::map<BoundaryID, SidesetInfo>>(
        "local_sideset_elems", REPORTER_MODE_DISTRIBUTED)),
    _sidesets(declareHelper<std::map<BoundaryID, SidesetInfo>>("sidesets", REPORTER_MODE_ROOT)),
    _sideset_elems(
        declareHelper<std::map<BoundaryID, SidesetInfo>>("sideset_elems", REPORTER_MODE_ROOT)),

    _local_subdomains(declareHelper<std::map<SubdomainID, SubdomainInfo>>(
        "local_subdomains", REPORTER_MODE_DISTRIBUTED)),
    _local_subdomain_elems(declareHelper<std::map<SubdomainID, SubdomainInfo>>(
        "local_subdomain_elems", REPORTER_MODE_DISTRIBUTED)),
    _subdomains(
        declareHelper<std::map<SubdomainID, SubdomainInfo>>("subdomains", REPORTER_MODE_ROOT)),
    _subdomain_elems(
        declareHelper<std::map<SubdomainID, SubdomainInfo>>("subdomain_elems", REPORTER_MODE_ROOT)),

    _equation_systems(_fe_problem.es()),
    _nonlinear_system(_fe_problem.es().get_system("nl0")),
    _aux_system(_fe_problem.es().get_system("aux0")),
    _mesh(_fe_problem.mesh().getMesh())
{
}

void
MeshInfo::execute()
{
  _num_dofs_nl = _nonlinear_system.n_dofs();
  _num_dofs_aux = _aux_system.n_dofs();
  _num_dofs = _equation_systems.n_dofs();
  _num_dofs_constrained = 0;
  for (auto s : make_range(_equation_systems.n_systems()))
    _num_dofs_constrained += _equation_systems.get_system(s).n_constrained_dofs();

  _num_node = _mesh.n_nodes();
  _num_elem = _mesh.n_elem();
  _num_local_dofs_nl = _nonlinear_system.n_local_dofs();
  _num_local_dofs_aux = _aux_system.n_local_dofs();
  _num_local_dofs = _num_local_dofs_nl + _num_local_dofs_aux;
  _num_local_node = _mesh.n_local_nodes();
  _num_local_elem = _mesh.n_local_elem();

  possiblyAddSidesetInfo();
  possiblyAddSubdomainInfo();
}

void
MeshInfo::possiblyAddSidesetInfo()
{
  // Helper for adding the sideset names to a given map of sidesets
  auto add_sideset_names = [&](std::map<BoundaryID, SidesetInfo> & sidesets)
  {
    for (auto & pair : sidesets)
      pair.second.name = _mesh.get_boundary_info().get_sideset_name(pair.second.id);
  };

  // Helper for sorting all of the sides in each sideset
  auto sort_sides = [](std::map<BoundaryID, SidesetInfo> & sidesets)
  {
    for (auto & pair : sidesets)
      std::sort(pair.second.sides.begin(), pair.second.sides.end());
  };

  const bool include_all = !_items.isValid();

  if (include_all || _items.isValueSet("local_sidesets") ||
      _items.isValueSet("local_sideset_elems") || _items.isValueSet("sideset_elems"))
  {
    _local_sidesets.clear();
    _local_sideset_elems.clear();
    _sideset_elems.clear();

    // Fill the local sideset information; all cases need it
    std::map<BoundaryID, SidesetInfo> sidesets;
    for (const auto & bnd_elem :
         as_range(_fe_problem.mesh().bndElemsBegin(), _fe_problem.mesh().bndElemsEnd()))
      if (bnd_elem->_elem->processor_id() == processor_id())
      {
        auto & entry = sidesets[bnd_elem->_bnd_id];
        entry.id = bnd_elem->_bnd_id;
        entry.sides.emplace_back(bnd_elem->_elem->id(), bnd_elem->_side);
      }

    // For local sidesets: copy over the local info, remove the sides, and add the names
    if (include_all || _items.isValueSet("local_sidesets"))
    {
      // Copy over the local sideset info, remove the sides, and add the names
      _local_sidesets = sidesets;
      for (auto & pair : _local_sidesets)
        pair.second.sides.clear();
      add_sideset_names(_local_sidesets);
    }

    // For local sideset elems: copy over the local info, and add the names
    if (include_all || _items.isValueSet("local_sideset_elems"))
    {
      _local_sideset_elems = sidesets;
      sort_sides(_local_sideset_elems);
      add_sideset_names(_local_sideset_elems);
    }

    // For the global sideset elems, we need to communicate all of the elems
    if (include_all || _items.isValueSet("sideset_elems"))
    {
      // Set up a structure for sending each (id, elem id, side) tuple to root
      std::map<processor_id_type,
               std::vector<std::tuple<boundary_id_type, dof_id_type, unsigned int>>>
          send_info;
      auto & root_info = send_info[0];
      for (const auto & pair : sidesets)
        for (const auto & side : pair.second.sides)
          root_info.emplace_back(pair.second.id, side.first, side.second);

      // Take the received information and insert it into _sideset_elems
      auto accumulate_info =
          [this](processor_id_type,
                 const std::vector<std::tuple<boundary_id_type, dof_id_type, unsigned int>> & info)
      {
        for (const auto & tuple : info)
        {
          const auto id = std::get<0>(tuple);
          auto & entry = _sideset_elems[id];
          entry.id = id;
          entry.sides.emplace_back(std::get<1>(tuple), std::get<2>(tuple));
        }
      };

      // Push the information and insert it into _sideset_elems on root
      Parallel::push_parallel_vector_data(comm(), send_info, accumulate_info);

      sort_sides(_sideset_elems);
      add_sideset_names(_sideset_elems);
    }
  }

  // For global sideset information without elements, we can simplify communication.
  // All we need are the boundary IDs from libMesh (may not be reduced, so take the union)
  // and then add the names (global)
  if (include_all || _items.isValueSet("sidesets"))
  {
    _sidesets.clear();

    auto boundary_ids = _mesh.get_boundary_info().get_boundary_ids();
    comm().set_union(boundary_ids, 0);
    if (processor_id() == 0)
    {
      for (const auto id : boundary_ids)
        _sidesets[id].id = id;
      add_sideset_names(_sidesets);
    }
  }
}

void
to_json(nlohmann::json & json, const std::map<BoundaryID, MeshInfo::SidesetInfo> & sidesets)
{
  for (const auto & pair : sidesets)
  {
    const MeshInfo::SidesetInfo & sideset_info = pair.second;

    nlohmann::json sideset_json;
    sideset_json["id"] = sideset_info.id;
    if (sideset_info.name.size())
      sideset_json["name"] = sideset_info.name;
    if (sideset_info.sides.size())
    {
      auto & sides_json = sideset_json["sides"];

      for (const std::pair<dof_id_type, unsigned int> & pair : sideset_info.sides)
      {
        nlohmann::json side_json;
        side_json["elem_id"] = pair.first;
        side_json["side"] = pair.second;
        sides_json.push_back(side_json);
      }
    }

    json.push_back(sideset_json);
  }
}

void
dataStore(std::ostream & stream, MeshInfo::SidesetInfo & sideset_info, void * context)
{
  storeHelper(stream, sideset_info.id, context);
  storeHelper(stream, sideset_info.name, context);
  storeHelper(stream, sideset_info.sides, context);
}

void
dataLoad(std::istream & stream, MeshInfo::SidesetInfo & sideset_info, void * context)
{
  loadHelper(stream, sideset_info.id, context);
  loadHelper(stream, sideset_info.name, context);
  loadHelper(stream, sideset_info.sides, context);
}

void
MeshInfo::possiblyAddSubdomainInfo()
{
  // Helper for adding the subdomain names to a given map of subdomains
  auto add_subdomain_names = [&](std::map<SubdomainID, SubdomainInfo> & subdomains)
  {
    for (auto & pair : subdomains)
      pair.second.name = _mesh.subdomain_name(pair.second.id);
  };

  // Helper for sorting all of the elems in each subdomain
  auto sort_elems = [](std::map<SubdomainID, SubdomainInfo> & subdomains)
  {
    for (auto & pair : subdomains)
      std::sort(pair.second.elems.begin(), pair.second.elems.end());
  };

  const bool include_all = !_items.isValid();

  if (include_all || _items.isValueSet("local_subdomains") ||
      _items.isValueSet("local_subdomain_elems") || _items.isValueSet("subdomain_elems"))
  {
    _local_subdomains.clear();
    _local_subdomain_elems.clear();
    _subdomain_elems.clear();

    // Fill the local subdomain information; all cases need it
    std::map<SubdomainID, SubdomainInfo> subdomains;
    for (const auto & elem : *_fe_problem.mesh().getActiveLocalElementRange())
    {
      auto & entry = subdomains[elem->subdomain_id()];
      entry.id = elem->subdomain_id();
      entry.elems.push_back(elem->id());
    }

    // For local subdomains: copy over the local info, remove the elems, and add the names
    if (include_all || _items.isValueSet("local_subdomains"))
    {
      _local_subdomains = subdomains;
      for (auto & pair : _local_subdomains)
        pair.second.elems.clear();
      add_subdomain_names(_local_subdomains);
    }

    // For local subdomain elems: copy over the local info, and add the names
    if (include_all || _items.isValueSet("local_subdomain_elems"))
    {
      _local_subdomain_elems = subdomains;
      sort_elems(_local_subdomain_elems);
      add_subdomain_names(_local_subdomain_elems);
    }

    // For the global subdomain elems, we need to communicate all of the elems
    if (include_all || _items.isValueSet("subdomain_elems"))
    {
      // Set up a structure for sending each (id, elem id) to root
      std::map<processor_id_type, std::vector<std::pair<subdomain_id_type, dof_id_type>>> send_info;
      auto & root_info = send_info[0];
      for (const auto & pair : subdomains)
        for (const auto elem_id : pair.second.elems)
          root_info.emplace_back(pair.second.id, elem_id);

      // Take the received information and insert it into _subdomain_elems
      auto accumulate_info =
          [this](processor_id_type,
                 const std::vector<std::pair<subdomain_id_type, dof_id_type>> & info)
      {
        for (const auto & subdomain_elem_pair : info)
        {
          auto & entry = _subdomain_elems[subdomain_elem_pair.first];
          entry.id = subdomain_elem_pair.first;
          entry.elems.emplace_back(subdomain_elem_pair.second);
        }
      };

      // Push the information and insert it into _subdomain_elems on root
      Parallel::push_parallel_vector_data(comm(), send_info, accumulate_info);

      if (processor_id() == 0)
      {
        sort_elems(_subdomain_elems);
        add_subdomain_names(_subdomain_elems);
      }
    }
  }

  // For global subdomain information without elements, we can simplify communication.
  // All we need are the subdomain IDs from libMesh and then add the names (global)
  if (include_all || _items.isValueSet("subdomains"))
  {
    _subdomains.clear();

    std::set<subdomain_id_type> subdomain_ids;
    _mesh.subdomain_ids(subdomain_ids);

    if (processor_id() == 0)
    {
      for (const auto id : subdomain_ids)
        _subdomains[id].id = id;
      add_subdomain_names(_subdomains);
    }
  }
}

void
to_json(nlohmann::json & json, const std::map<SubdomainID, MeshInfo::SubdomainInfo> & subdomains)
{
  for (const auto & pair : subdomains)
  {
    const MeshInfo::SubdomainInfo & subdomain_info = pair.second;

    nlohmann::json subdomain_json;
    subdomain_json["id"] = subdomain_info.id;
    if (subdomain_info.name.size())
      subdomain_json["name"] = subdomain_info.name;
    if (subdomain_info.elems.size())
    {
      auto & sides_json = subdomain_json["elems"];
      for (const auto & id : subdomain_info.elems)
        sides_json.push_back(id);
    }

    json.push_back(subdomain_json);
  }
}

void
dataStore(std::ostream & stream, MeshInfo::SubdomainInfo & subdomain_info, void * context)
{
  storeHelper(stream, subdomain_info.id, context);
  storeHelper(stream, subdomain_info.name, context);
  storeHelper(stream, subdomain_info.elems, context);
}

void
dataLoad(std::istream & stream, MeshInfo::SubdomainInfo & subdomain_info, void * context)
{
  loadHelper(stream, subdomain_info.id, context);
  loadHelper(stream, subdomain_info.name, context);
  loadHelper(stream, subdomain_info.elems, context);
}
