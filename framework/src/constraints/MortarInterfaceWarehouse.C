//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarInterfaceWarehouse.h"
#include "SubProblem.h"
#include "MooseMesh.h"
#include "MooseError.h"
#include "MooseEnum.h"
#include "MooseUtils.h"
#include "MortarExecutorInterface.h"
#include "AutomaticMortarGeneration.h"

namespace
{
MortarSegmentTriangulationMode
toTriangulationMode(const MooseEnum & triangulation)
{
  if (triangulation == "vertex")
    return MortarSegmentTriangulationMode::Vertex;
  if (triangulation == "centroid")
    return MortarSegmentTriangulationMode::Centroid;
  if (triangulation == "ear_clipping")
    return MortarSegmentTriangulationMode::EarClipping;
#if defined(LIBMESH_HAVE_TRIANGLE) || defined(LIBMESH_HAVE_POLY2TRI)
  if (triangulation == "delaunay")
    return MortarSegmentTriangulationMode::Delaunay;
#endif
  mooseError("Unsupported mortar triangulation option: ", triangulation);
}
}

MortarInterfaceWarehouse::MortarInterfaceWarehouse(const libMesh::ParallelObject & other)
  : libMesh::ParallelObject(other), _mortar_initd(false)
{
}

void
MortarInterfaceWarehouse::createMortarInterface(
    const std::pair<BoundaryID, BoundaryID> & boundary_key,
    const std::pair<SubdomainID, SubdomainID> & subdomain_key,
    SubProblem & subproblem,
    bool on_displaced,
    bool periodic,
    const bool debug,
    const bool correct_edge_dropping,
    const Real minimum_projection_angle,
    const Mortar3DSubpatchPlane mortar_3d_subpatch_plane,
    const MooseEnum & triangulation,
    const bool triangulate_triangles)
{
  _mortar_subdomain_coverage.insert(subdomain_key.first);
  _mortar_subdomain_coverage.insert(subdomain_key.second);

  _mortar_boundary_coverage.insert(boundary_key.first);
  _mortar_boundary_coverage.insert(boundary_key.second);

  MeshBase & mesh = subproblem.mesh().getMesh();

  auto & mortar_interfaces = on_displaced ? _displaced_mortar_interfaces : _mortar_interfaces;
  const auto triangulation_mode = toTriangulationMode(triangulation);

  auto interface_iterator = mortar_interfaces.find(boundary_key);
  if (interface_iterator != mortar_interfaces.end())
  {
    // Existing entry: every per-interface flag must agree across constraints sharing the same
    // primary-secondary surface pair.
    const auto & existing = interface_iterator->second;
    if (existing.periodic != periodic)
      mooseError("We do not currently support enforcing both periodic and non-periodic constraints "
                 "on the same boundary primary-secondary pair");
    if (existing.debug != debug)
      mooseError(
          "We do not currently support generating and not generating debug output "
          "on the same boundary primary-secondary surface pair. Please set debug_mesh = true for "
          "all constraints sharing the same primary-secondary surface pairs");
    if (!MooseUtils::absoluteFuzzyEqual(existing.minimum_projection_angle,
                                        minimum_projection_angle))
      mooseError("We do not currently support multiple values of 'minimum_projection_angle' on "
                 "the same boundary primary-secondary surface pair.");
    if (existing.mortar_3d_subpatch_plane != mortar_3d_subpatch_plane)
      mooseError("Mortar constraints sharing the same primary/secondary mortar interface must use "
                 "the same 'mortar_3d_subpatch_plane' value.");
    if (existing.triangulation != triangulation_mode)
      mooseError("We do not currently support multiple values of 'triangulation' on the same "
                 "boundary primary-secondary surface pair.");
    if (existing.triangulate_triangles != triangulate_triangles)
      mooseError("We do not currently support multiple values of 'triangulate_triangles' on the "
                 "same boundary primary-secondary surface pair.");
  }
  else
  {
    MortarInterfaceConfig config{
        std::make_unique<AutomaticMortarGeneration>(subproblem.getMooseApp(),
                                                    mesh,
                                                    boundary_key,
                                                    subdomain_key,
                                                    on_displaced,
                                                    periodic,
                                                    debug,
                                                    correct_edge_dropping,
                                                    minimum_projection_angle,
                                                    mortar_3d_subpatch_plane,
                                                    triangulation_mode,
                                                    triangulate_triangles),
        periodic,
        debug,
        minimum_projection_angle,
        mortar_3d_subpatch_plane,
        triangulation_mode,
        triangulate_triangles};
    config.amg->initOutput();
    mortar_interfaces.emplace(boundary_key, std::move(config));
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
MortarInterfaceWarehouse::getMortarInterface(
    const std::pair<BoundaryID, BoundaryID> & boundary_key,
    const std::pair<SubdomainID, SubdomainID> & /*subdomain_key*/,
    bool on_displaced) const
{
  auto & mortar_interfaces = on_displaced ? _displaced_mortar_interfaces : _mortar_interfaces;
  auto it = mortar_interfaces.find(boundary_key);
  if (it == mortar_interfaces.end())
    mooseError(
        "The requested mortar interface AutomaticMortarGeneration object does not yet exist!");
  return *it->second.amg;
}

AutomaticMortarGeneration &
MortarInterfaceWarehouse::getMortarInterface(
    const std::pair<BoundaryID, BoundaryID> & boundary_key,
    const std::pair<SubdomainID, SubdomainID> & subdomain_key,
    bool on_displaced)
{
  return const_cast<AutomaticMortarGeneration &>(
      const_cast<const MortarInterfaceWarehouse *>(this)->getMortarInterface(
          boundary_key, subdomain_key, on_displaced));
}

void
MortarInterfaceWarehouse::update()
{
  for (auto & mortar_pair : _mortar_interfaces)
    update(*mortar_pair.second.amg);
  for (auto & mortar_pair : _displaced_mortar_interfaces)
    update(*mortar_pair.second.amg);

  _mortar_initd = true;
}

void
MortarInterfaceWarehouse::meshChanged()
{
  for (auto & mortar_pair : _mortar_interfaces)
    mortar_pair.second.amg->meshChanged();
  for (auto & mortar_pair : _displaced_mortar_interfaces)
    mortar_pair.second.amg->meshChanged();
  update();
}

void
MortarInterfaceWarehouse::update(AutomaticMortarGeneration & amg)
{
  // Clear exiting data
  amg.clear();

  const auto dim = amg.dim();

  if (dim == 1)
    mooseError("Mortar constraints are not currently supported for 1D meshes");
  else if (dim != 2 && dim != 3)
    mooseError("Invalid mesh dimension for mortar constraint");

  // Construct maps from nodes -> lower dimensional elements on the primary and secondary
  // boundaries.
  amg.buildNodeToElemMaps();

  // Compute nodal geometry (normals and tangents).
  amg.computeNodalGeometry();

  if (dim == 2)
  {
    // Project secondary nodes (find xi^(2) values).
    amg.projectSecondaryNodes();

    // Project primary nodes (find xi^(1) values).
    amg.projectPrimaryNodes();

    // Build the mortar segment mesh on the secondary boundary.
    amg.buildMortarSegmentMesh();
  }
  else // dim == 3
    amg.buildMortarSegmentMesh3d();

  amg.computeInactiveLMNodes();
  amg.computeInactiveLMElems();
}

const std::set<SubdomainID> &
MortarInterfaceWarehouse::getHigherDimSubdomainIDs(SubdomainID lower_d_subdomain_id) const
{
  if (_lower_d_sub_to_higher_d_subs.find(lower_d_subdomain_id) ==
      _lower_d_sub_to_higher_d_subs.end())
    mooseError("The lower dimensional ID ",
               lower_d_subdomain_id,
               " has not been added to MortarInterfaceWarehouse yet");
  return _lower_d_sub_to_higher_d_subs.at(lower_d_subdomain_id);
}

void
MortarInterfaceWarehouse::notifyWhenMortarSetup(MortarExecutorInterface * const mei_obj)
{
  _mei_objs.insert(mei_obj);
}

void
MortarInterfaceWarehouse::dontNotifyWhenMortarSetup(MortarExecutorInterface * const mei_obj)
{
  _mei_objs.erase(mei_obj);
}
