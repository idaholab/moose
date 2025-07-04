//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SidesetAroundSubdomainUpdater.h"
#include "DisplacedProblem.h"
#include "Assembly.h"

#include "libmesh/parallel_algebra.h"
#include "libmesh/parallel_sync.h"

registerMooseObject("MooseApp", SidesetAroundSubdomainUpdater);

InputParameters
SidesetAroundSubdomainUpdater::validParams()
{
  InputParameters params = DomainUserObject::validParams();
  params.addClassDescription("Sets up a boundary between inner_subdomains and outer_subdomains");
  params.addParam<std::vector<SubdomainName>>("inner_subdomains",
                                              "Subdomains that own the boundary");
  params.addParam<std::vector<SubdomainName>>("outer_subdomains",
                                              "Subdomains on the outside of the boundary");
  params.addParam<BoundaryName>("mask_side",
                                "If specified, only add sides where this sideset exists.");
  params.addParam<bool>("assign_outer_surface_sides",
                        true,
                        "Assign sides of elements im `inner_subdomains` that have no neighbor.");
  params.addRequiredParam<BoundaryName>("update_sideset_name",
                                        "The name of the sideset to be updated. If the boundary "
                                        "does not exist it will be added to the system.");
  params.renameParam(
      "update_sideset_name", "update_boundary_name", "The boundary name which is updated.");
  params.addParam<BoundaryID>("update_sideset_id",
                              Moose::INVALID_BOUNDARY_ID,
                              "The ID of the sideset to be updated. If the boundary "
                              "does not exist it will be added to the system.");
  params.renameParam(
      "update_sideset_id", "update_boundary_id", "The boundary id which is updated.");
  params.registerBase("MeshModifier");
  return params;
}

SidesetAroundSubdomainUpdater::SidesetAroundSubdomainUpdater(const InputParameters & parameters)
  : DomainUserObject(parameters),
    _pid(_communicator.rank()),
    _displaced_problem(_fe_problem.getDisplacedProblem().get()),
    _neighbor_side(_assembly.neighborSide()),
    _assign_outer_surface_sides(getParam<bool>("assign_outer_surface_sides")),
    _boundary_name(getParam<BoundaryName>("update_boundary_name")),
    _boundary_id(_mesh.getBoundaryID(_boundary_name)),
    _mask_side(isParamValid("mask_side") ? _mesh.getBoundaryID(getParam<BoundaryName>("mask_side"))
                                         : Moose::INVALID_BOUNDARY_ID),
    _boundary_info(_mesh.getMesh().get_boundary_info()),
    _displaced_boundary_info(
        _displaced_problem ? &_displaced_problem->mesh().getMesh().get_boundary_info() : nullptr)
{
  // subdomains
  const auto & inner_subdomains = getParam<std::vector<SubdomainName>>("inner_subdomains");
  const auto & outer_subdomains = getParam<std::vector<SubdomainName>>("outer_subdomains");
  for (const auto id : MooseMeshUtils::getSubdomainIDs(_mesh.getMesh(), inner_subdomains))
    _inner_ids.insert(id);
  for (const auto id : MooseMeshUtils::getSubdomainIDs(_mesh.getMesh(), outer_subdomains))
    _outer_ids.insert(id);
  std::set<SubdomainID> mesh_subdomains = _mesh.meshSubdomains();

  for (const auto & id : _inner_ids)
    if (mesh_subdomains.find(id) == mesh_subdomains.end())
      paramError("inner_subdomains", "The block '", id, "' was not found in the mesh");
  for (const auto & id : _outer_ids)
    if (mesh_subdomains.find(id) == mesh_subdomains.end())
      paramError("outer_subdomains", "The block '", id, "' was not found in the mesh");

  // save boundary name
  _boundary_info.sideset_name(_boundary_id) = _boundary_name;
  _boundary_info.nodeset_name(_boundary_id) = _boundary_name;
  if (_displaced_boundary_info)
  {
    _displaced_boundary_info->sideset_name(_boundary_id) = _boundary_name;
    _displaced_boundary_info->nodeset_name(_boundary_id) = _boundary_name;
  }
  if (!_mesh.getConstructNodeListFromSideList())
    mooseDoOnce(
        mooseWarning("The user has selected 'construct_node_list_from_side_list' as false, but "
                     "SidesetAroundSubdomainUpdate is building node lists from the side lists."));
}

void
SidesetAroundSubdomainUpdater::executeOnExternalSide(const Elem * elem, unsigned int side)
{
  // we should add the sideset only of the current element is in the "inner" set _and_ the user set
  // assign_surface_sides
  if (_inner_ids.count(elem->subdomain_id()))
  {
    if (_assign_outer_surface_sides && !_boundary_info.has_boundary_id(elem, side, _boundary_id) &&
        (_mask_side == Moose::INVALID_BOUNDARY_ID ||
         _boundary_info.has_boundary_id(elem, side, _mask_side)))
      _add[_pid].emplace_back(elem->id(), side);
  }
  else
  {
    if (_boundary_info.has_boundary_id(elem, side, _boundary_id))
      _remove[_pid].emplace_back(elem->id(), side);
  }
}

void
SidesetAroundSubdomainUpdater::executeOnInternalSide()
{
  processSide(_current_elem, _current_side, _neighbor_elem);
  processSide(_neighbor_elem, _neighbor_side, _current_elem);
}

void
SidesetAroundSubdomainUpdater::processSide(const Elem * primary_elem,
                                           unsigned short int primary_side,
                                           const Elem * secondary_elem)
{
  // undisplaced mesh
  if (_inner_ids.count(primary_elem->subdomain_id()) &&
      _outer_ids.count(secondary_elem->subdomain_id()))
  {

    // we are on an inner element facing an outer element->add boundary
    if ((!_boundary_info.has_boundary_id(primary_elem, primary_side, _boundary_id)) &&
        (_mask_side == Moose::INVALID_BOUNDARY_ID ||
         _boundary_info.has_boundary_id(primary_elem, primary_side, _mask_side)))
      _add[primary_elem->processor_id()].emplace_back(primary_elem->id(), primary_side);
  }
  else
  {
    // we are on an outer, between inner elements, or other elements etc., delete
    // the boundary for sure
    if (_boundary_info.has_boundary_id(primary_elem, primary_side, _boundary_id))
      _remove[primary_elem->processor_id()].emplace_back(primary_elem->id(), primary_side);
  }
}

void
SidesetAroundSubdomainUpdater::initialize()
{
  _add.clear();
  _remove.clear();
}

void
SidesetAroundSubdomainUpdater::threadJoin(const UserObject & uo)
{
  const auto & sas = static_cast<const SidesetAroundSubdomainUpdater &>(uo);

  for (const auto & [pid, list] : sas._add)
    _add[pid].insert(_add[pid].end(), list.begin(), list.end());

  for (const auto & [pid, list] : sas._remove)
    _remove[pid].insert(_remove[pid].end(), list.begin(), list.end());
}

void
SidesetAroundSubdomainUpdater::finalize()
{
  const auto & mesh = _mesh.getMesh();
  const auto * displaced_mesh =
      _displaced_problem ? &_displaced_problem->mesh().getMesh() : nullptr;

  auto add_functor = [this, &mesh, &displaced_mesh](const processor_id_type, const auto & sent_data)
  {
    for (auto & [elem_id, side] : sent_data)
    {
      _boundary_info.add_side(mesh.elem_ptr(elem_id), side, _boundary_id);
      if (_displaced_boundary_info)
        _displaced_boundary_info->add_side(displaced_mesh->elem_ptr(elem_id), side, _boundary_id);
    }
  };

  auto remove_functor =
      [this, &mesh, &displaced_mesh](const processor_id_type, const SideList & sent_data)
  {
    for (const auto & [elem_id, side] : sent_data)
    {
      const auto elem = mesh.elem_ptr(elem_id);
      _boundary_info.remove_side(elem, side, _boundary_id);
      for (const auto local_node_id : elem->nodes_on_side(side))
        _boundary_info.remove_node(elem->node_ptr(local_node_id), _boundary_id);
      if (_displaced_boundary_info)
      {
        const auto displaced_elem = displaced_mesh->elem_ptr(elem_id);
        _displaced_boundary_info->remove_side(displaced_elem, side, _boundary_id);
        for (const auto local_node_id : displaced_elem->nodes_on_side(side))
          _displaced_boundary_info->remove_node(displaced_elem->node_ptr(local_node_id),
                                                _boundary_id);
      }
    }
  };

  // communicate and act on remote and local changes
  TIMPI::push_parallel_vector_data(_communicator, _remove, remove_functor);
  TIMPI::push_parallel_vector_data(_communicator, _add, add_functor);

  auto sync = [](auto & mesh)
  {
    mesh.getMesh().get_boundary_info().parallel_sync_side_ids();
    mesh.getMesh().get_boundary_info().parallel_sync_node_ids();
    mesh.getMesh().get_boundary_info().build_node_list_from_side_list();
    mesh.update();
  };
  sync(_mesh);
  if (_displaced_problem)
    sync(_displaced_problem->mesh());

  // Reinit equation systems
  _fe_problem.meshChanged(
      /*intermediate_change=*/false, /*contract_mesh=*/false, /*clean_refinement_flags=*/false);
}
