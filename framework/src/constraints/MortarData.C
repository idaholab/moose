//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarData.h"
#include "SubProblem.h"
#include "MooseMesh.h"
#include "MooseError.h"
#include "MortarExecutorInterface.h"

MortarData::MortarData(const libMesh::ParallelObject & other)
  : libMesh::ParallelObject(other), _mortar_initd(false)
{
}

void
MortarData::createMortarInterface(const std::pair<BoundaryID, BoundaryID> & boundary_key,
                                  const std::pair<SubdomainID, SubdomainID> & subdomain_key,
                                  SubProblem & subproblem,
                                  bool on_displaced,
                                  bool periodic,
                                  const bool debug,
                                  const bool correct_edge_dropping,
                                  const Real minimum_projection_angle)
{
  _mortar_subdomain_coverage.insert(subdomain_key.first);
  _mortar_subdomain_coverage.insert(subdomain_key.second);

  _mortar_boundary_coverage.insert(boundary_key.first);
  _mortar_boundary_coverage.insert(boundary_key.second);

  MeshBase & mesh = subproblem.mesh().getMesh();

  auto & periodic_map = on_displaced ? _displaced_periodic_map : _periodic_map;
  auto & debug_flag_map = on_displaced ? _displaced_debug_flag_map : _debug_flag_map;
  auto & mortar_interfaces = on_displaced ? _displaced_mortar_interfaces : _mortar_interfaces;

  // Periodic flag
  auto periodic_map_iterator = periodic_map.find(boundary_key);
  if (periodic_map_iterator != periodic_map.end() && periodic_map_iterator->second != periodic)
    mooseError("We do not currently support enforcing both periodic and non-periodic constraints "
               "on the same boundary primary-secondary pair");
  else
    periodic_map.insert(periodic_map_iterator, std::make_pair(boundary_key, periodic));

  // Debug mesh flag displaced
  auto debug_flag_map_iterator = debug_flag_map.find(boundary_key);
  if (debug_flag_map_iterator != debug_flag_map.end() && debug_flag_map_iterator->second != debug)
    mooseError(
        "We do not currently support generating and not generating debug output "
        "on the same boundary primary-secondary surface pair. Please set debug_mesh = true for "
        "all constraints sharing the same primary-secondary surface pairs");
  else
    debug_flag_map.insert(debug_flag_map_iterator, std::make_pair(boundary_key, debug));

  // Generate lower-d mesh
  if (mortar_interfaces.find(boundary_key) == mortar_interfaces.end())
  {
    auto [it, inserted] =
        mortar_interfaces.emplace(boundary_key,
                                  AutomaticMortarGeneration(subproblem.getMooseApp(),
                                                            mesh,
                                                            boundary_key,
                                                            subdomain_key,
                                                            on_displaced,
                                                            periodic,
                                                            debug,
                                                            correct_edge_dropping,
                                                            minimum_projection_angle));
    if (inserted)
      it->second.initOutput();
  }

  // See whether to query the mesh
  SubdomainID key1 = subdomain_key.first;
  SubdomainID key2 = subdomain_key.second;

  // it(1,2) is a a pair consisting of an iterator to the inserted element (or to the element that
  // prevented the insertion) and a bool denoting whether the insertion took place.
  auto it1 = _lower_d_sub_to_higher_d_subs.insert(std::make_pair(key1, std::set<SubdomainID>{}));
  auto it2 = _lower_d_sub_to_higher_d_subs.insert(std::make_pair(key2, std::set<SubdomainID>{}));

  // Each entry in this vector will be a pair. The first member of the pair corresponds to
  // the lower dimensional subomain ID. The second member of the pair corresponds to the higher
  // dimensional subdomain ids of the lower dimeionsal interior parents
  std::vector<std::pair<SubdomainID, std::set<SubdomainID> *>> subdomains_to_probe;

  if (it1.second)
    subdomains_to_probe.push_back(std::make_pair(key1, &it1.first->second));
  if (it2.second)
    subdomains_to_probe.push_back(std::make_pair(key2, &it2.first->second));

  for (auto & pr : subdomains_to_probe)
  {
    for (const Elem * lower_d_elem : as_range(mesh.active_local_subdomain_elements_begin(pr.first),
                                              mesh.active_local_subdomain_elements_end(pr.first)))
    {
      const Elem * ip = lower_d_elem->interior_parent();
      mooseAssert(
          ip,
          "Lower dimensional elements should always have an interior parent set when using mortar");
      pr.second->insert(ip->subdomain_id());
    }

    // Make sure that we get this right in parallel
    _communicator.set_union(*pr.second);
  }
}

const AutomaticMortarGeneration &
MortarData::getMortarInterface(const std::pair<BoundaryID, BoundaryID> & boundary_key,
                               const std::pair<SubdomainID, SubdomainID> & /*subdomain_key*/,
                               bool on_displaced) const
{
  if (on_displaced)
  {
    if (_displaced_mortar_interfaces.find(boundary_key) == _displaced_mortar_interfaces.end())
      mooseError(
          "The requested mortar interface AutomaticMortarGeneration object does not yet exist!");

    return _displaced_mortar_interfaces.at(boundary_key);
  }
  else
  {
    if (_mortar_interfaces.find(boundary_key) == _mortar_interfaces.end())
      mooseError(
          "The requested mortar interface AutomaticMortarGeneration object does not yet exist!");

    return _mortar_interfaces.at(boundary_key);
  }
}

AutomaticMortarGeneration &
MortarData::getMortarInterface(const std::pair<BoundaryID, BoundaryID> & boundary_key,
                               const std::pair<SubdomainID, SubdomainID> & subdomain_key,
                               bool on_displaced)
{
  return const_cast<AutomaticMortarGeneration &>(
      const_cast<const MortarData *>(this)->getMortarInterface(
          boundary_key, subdomain_key, on_displaced));
}

void
MortarData::update()
{
  for (auto & mortar_pair : _mortar_interfaces)
    update(mortar_pair.second);
  for (auto & mortar_pair : _displaced_mortar_interfaces)
    update(mortar_pair.second);

  _mortar_initd = true;
}

void
MortarData::update(AutomaticMortarGeneration & amg)
{
  // Clear exiting data
  amg.clear();

  // Construct maps from nodes -> lower dimensional elements on the primary and secondary
  // boundaries.
  amg.buildNodeToElemMaps();

  // Compute nodal geometry (normals and tangents).
  amg.computeNodalGeometry();

  const auto dim = amg.dim();
  if (dim == 2)
  {
    // Project secondary nodes (find xi^(2) values).
    amg.projectSecondaryNodes();

    // Project primary nodes (find xi^(1) values).
    amg.projectPrimaryNodes();

    // Build the mortar segment mesh on the secondary boundary.
    amg.buildMortarSegmentMesh();
  }
  else if (dim == 3)
    amg.buildMortarSegmentMesh3d();
  else
    mooseError("Invalid mesh dimension for mortar constraint");

  amg.computeInactiveLMNodes();
  amg.computeInactiveLMElems();
}

const std::set<SubdomainID> &
MortarData::getHigherDimSubdomainIDs(SubdomainID lower_d_subdomain_id) const
{
  if (_lower_d_sub_to_higher_d_subs.find(lower_d_subdomain_id) ==
      _lower_d_sub_to_higher_d_subs.end())
    mooseError(
        "The lower dimensional ID ", lower_d_subdomain_id, " has not been added to MortarData yet");
  return _lower_d_sub_to_higher_d_subs.at(lower_d_subdomain_id);
}

void
MortarData::notifyWhenMortarSetup(MortarExecutorInterface * const mei_obj)
{
  _mei_objs.insert(mei_obj);
}

void
MortarData::dontNotifyWhenMortarSetup(MortarExecutorInterface * const mei_obj)
{
  _mei_objs.erase(mei_obj);
}
