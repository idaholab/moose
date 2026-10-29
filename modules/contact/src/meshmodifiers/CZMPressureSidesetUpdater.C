//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CZMPressureSidesetUpdater.h"
#include "MooseMesh.h"
#include "DisplacedProblem.h"
#include "AutomaticMortarGeneration.h"

registerMooseObject("ContactApp", CZMPressureSidesetUpdater);

InputParameters
CZMPressureSidesetUpdater::validParams()
{
  InputParameters params = DomainUserObject::validParams();
  params.addClassDescription("Promotes fully-damaged CZM interface segments into new sidesets.");

  params.addRequiredCoupledVar("damage_variable",
                               "CZM damage AuxVariable (0 = intact, 1 = fully broken), valid on "
                               "the secondary lower-dimensional mortar block.");
  params.addParam<Real>(
      "threshold", 1.0 - 1e-10, "Damage at or above which a segment is fully separated.");
  params.addRequiredParam<UserObjectName>(
      "cohesive_zone_userobject", "The mortar cohesive-zone UserObject for this interface.");
  params.addRequiredParam<BoundaryName>("primary_boundary",
                                        "Primary side of the CZM mortar interface.");
  params.addRequiredParam<BoundaryName>("secondary_boundary",
                                        "Secondary side of the CZM mortar interface.");
  params.addRequiredParam<SubdomainName>("primary_subdomain",
                                         "Primary lower-dimensional mortar block.");
  params.addRequiredParam<SubdomainName>("secondary_subdomain",
                                         "Secondary lower-dimensional mortar block.");
  params.addRequiredParam<BoundaryName>("pressure_boundary_primary",
                                        "Boundary to populate with fully-damaged primary faces.");
  params.addRequiredParam<BoundaryName>("pressure_boundary_secondary",
                                        "Boundary to populate with fully-damaged secondary faces.");
  params.set<bool>("use_displaced_mesh") = true;
  params.registerBase("MeshModifier");
  return params;
}

CZMPressureSidesetUpdater::CZMPressureSidesetUpdater(const InputParameters & parameters)
  : DomainUserObject(parameters),
    _damage(coupledValue("damage_variable")),
    _threshold(getParam<Real>("threshold")),
    _czm_userobject_name(getParam<UserObjectName>("cohesive_zone_userobject")),
    _primary_boundary_id(_mesh.getBoundaryID(getParam<BoundaryName>("primary_boundary"))),
    _secondary_boundary_id(_mesh.getBoundaryID(getParam<BoundaryName>("secondary_boundary"))),
    _primary_subdomain_id(_mesh.getSubdomainID(getParam<SubdomainName>("primary_subdomain"))),
    _secondary_subdomain_id(_mesh.getSubdomainID(getParam<SubdomainName>("secondary_subdomain"))),
    _displaced_problem(_fe_problem.getDisplacedProblem().get()),
    _pressure_name_primary(getParam<BoundaryName>("pressure_boundary_primary")),
    _pressure_name_secondary(getParam<BoundaryName>("pressure_boundary_secondary")),
    _pressure_bid_primary(_mesh.getBoundaryID(_pressure_name_primary)),
    _pressure_bid_secondary(_mesh.getBoundaryID(_pressure_name_secondary)),
    _boundary_info(_fe_problem.mesh().getMesh().get_boundary_info()),
    _displaced_boundary_info(
        _displaced_problem ? &_displaced_problem->mesh().getMesh().get_boundary_info() : nullptr)
{
  // Mortar machinery lives on the displaced mesh, so this object must execute there.
  if (!_displaced_problem)
    mooseError("CZMPressureSidesetUpdater requires a displaced mesh.");

  // Persist boundary names on both undisplaced and displaced meshes.
  _boundary_info.sideset_name(_pressure_bid_primary) = _pressure_name_primary;
  _boundary_info.nodeset_name(_pressure_bid_primary) = _pressure_name_primary;
  _boundary_info.sideset_name(_pressure_bid_secondary) = _pressure_name_secondary;
  _boundary_info.nodeset_name(_pressure_bid_secondary) = _pressure_name_secondary;

  _displaced_boundary_info->sideset_name(_pressure_bid_primary) = _pressure_name_primary;
  _displaced_boundary_info->nodeset_name(_pressure_bid_primary) = _pressure_name_primary;
  _displaced_boundary_info->sideset_name(_pressure_bid_secondary) = _pressure_name_secondary;
  _displaced_boundary_info->nodeset_name(_pressure_bid_secondary) = _pressure_name_secondary;
}

void
CZMPressureSidesetUpdater::initialSetup()
{
  if (_tid == 0)
    buildReferencePrimaryFaceMap();
}

void
CZMPressureSidesetUpdater::buildReferencePrimaryFaceMap()
{

  AutomaticMortarGeneration amg(_app,
                                _fe_problem.mesh().getMesh(),
                                std::make_pair(_primary_boundary_id, _secondary_boundary_id),
                                std::make_pair(_primary_subdomain_id, _secondary_subdomain_id),
                                /*on_displaced=*/false,
                                /*periodic=*/false,
                                /*debug=*/false,
                                /*correct_edge_dropping=*/true,
                                /*minimum_projection_angle=*/40.0,
                                Mortar3DSubpatchPlane::GEOMETRIC_NORMAL,
                                MortarSegmentTriangulationMode::Centroid,
                                /*triangulate_triangles=*/false);
  amg.buildNodeToElemMaps();
  amg.computeNodalGeometry();
  if (_mesh.dimension() == 2)
  {
    amg.projectSecondaryNodes();
    amg.projectPrimaryNodes();
    amg.buildMortarSegmentMesh();
  }
  else
    amg.buildMortarSegmentMesh3d();

  // Each mortar segment is the overlap of one secondary face with one primary face, so the set of
  // segments of a secondary face names every primary face it was bonded to.
  const auto & seg_to_info = amg.mortarSegmentMeshElemToInfo();
  _secondary_to_primary_faces.clear();
  for (const auto & [secondary_id, segments] : amg.secondariesToMortarSegments())
  {
    auto & faces = _secondary_to_primary_faces[secondary_id];
    for (const Elem * const mseg : segments)
    {
      const Elem * const primary_lowerd = libmesh_map_find(seg_to_info, mseg).primary_elem;
      if (!primary_lowerd)
        continue;

      const Elem * const primary_ip = primary_lowerd->interior_parent();
      mooseAssert(primary_ip, "Lower-d mortar element has no interior parent.");
      const std::pair<dof_id_type, unsigned int> face(primary_ip->id(),
                                                      primary_ip->which_side_am_i(primary_lowerd));
      if (std::find(faces.begin(), faces.end(), face) == faces.end())
        faces.push_back(face);
    }
  }
}

void
CZMPressureSidesetUpdater::initialize()
{
  _separated_lowerd.clear();
}

bool
CZMPressureSidesetUpdater::currentElemFullySeparated() const
{
  for (unsigned int qp = 0; qp < qRule().n_points(); ++qp)
    if (_damage[qp] < _threshold)
      return false;
  return true;
}

void
CZMPressureSidesetUpdater::executeOnElement()
{
  if (currentElemFullySeparated())
    _separated_lowerd.push_back(_current_elem->id());
}

void
CZMPressureSidesetUpdater::threadJoin(const UserObject & y)
{
  const auto & uo = static_cast<const CZMPressureSidesetUpdater &>(y);
  _separated_lowerd.insert(
      _separated_lowerd.end(), uo._separated_lowerd.begin(), uo._separated_lowerd.end());
}

void
CZMPressureSidesetUpdater::finalize()
{
  const auto & mesh = _fe_problem.mesh().getMesh();
  const auto & displaced_mesh = _displaced_problem->mesh().getMesh();

  // Sides not yet in their sideset, gathered on every rank so that all ranks add the same sides and
  // the boundary info of the replicated mesh stays consistent without a parallel sync. Only sides
  // that are actually new are collected, so that the mesh (and the Exodus output) is changed only
  // on the steps where a segment is first promoted.
  std::set<std::tuple<dof_id_type, unsigned short int, boundary_id_type>> new_sides;
  for (const auto lowerd_id : _separated_lowerd)
  {
    const Elem * const lowerd = mesh.elem_ptr(lowerd_id);
    const Elem * const secondary_ip = lowerd->interior_parent();
    mooseAssert(secondary_ip, "Lower-d mortar element has no interior parent.");
    const auto secondary_side = secondary_ip->which_side_am_i(lowerd);
    if (!_boundary_info.has_boundary_id(secondary_ip, secondary_side, _pressure_bid_secondary))
      new_sides.emplace(secondary_ip->id(), secondary_side, _pressure_bid_secondary);

    const auto it = _secondary_to_primary_faces.find(lowerd_id);
    if (it != _secondary_to_primary_faces.end())
      for (const auto & [primary_id, primary_side] : it->second)
        if (!_boundary_info.has_boundary_id(
                mesh.elem_ptr(primary_id), primary_side, _pressure_bid_primary))
          new_sides.emplace(primary_id, primary_side, _pressure_bid_primary);
  }
  _communicator.set_union(new_sides);

  if (new_sides.empty())
    return;

  for (const auto & [elem_id, side, bid] : new_sides)
  {
    _boundary_info.add_side(mesh.elem_ptr(elem_id), side, bid);
    _displaced_boundary_info->add_side(displaced_mesh.elem_ptr(elem_id), side, bid);
  }

  // Update both undisplaced and displaced meshes
  for (auto * const moose_mesh : {&_fe_problem.mesh(), &_displaced_problem->mesh()})
  {
    moose_mesh->getMesh().get_boundary_info().build_node_list_from_side_list();
    moose_mesh->update();
  }

  // Rebuild equation systems AND the mortar functor maps. This triggers reinit of
  // BCs and other objects so they see the newly-populated boundary sides.
  _fe_problem.meshChanged(
      /*intermediate_change=*/false, /*contract_mesh=*/false, /*clean_refinement_flags=*/false);
}
