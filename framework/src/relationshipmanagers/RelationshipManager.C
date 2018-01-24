//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RelationshipManager.h"

template <>
InputParameters
validParams<RelationshipManager>()
{
  InputParameters params = validParams<MooseObject>();

  /**
   * Param to indicate whether all necessary GeometricRelationshipManagers can be attached during
   * the setup of the mesh. When true (and when running with DistributedMesh) the framework can
   * perform two optimizations:
   *
   * 1) Elements not needed by any ghosting functor can be safely deleted during Mesh::init() prior
   *    to building the EquationSystems object or other MOOSE objects. This may result in
   *    significant memory savings during the setup phase.
   *
   * 2) A late mesh reinitialization can be skipped reducing the overall setup time of the
   *    simulation.
   *
   * When in doubt and to allow for maximum flexibility, developers may choose to return false here
   * to simplify setting up Algebraic and Geometric at the same time.
   */
  params.addPrivateParam<bool>("attach_geometric_early", true);

  /**
   * This parameter indicates which type of RelationshipManager this class represents prior to its
   * construction. Geometric-only RelationshipManagers may be buildable early in which case the
   * framework can optimize memory usage for Distributed Mesh by allowing remote elements to be
   * deleted. Depending on the mesh size and EquationSystems size, this may have a significant
   * impact on total memory usage during the problem setup phase.
   */
  params.addPrivateParam<Moose::RelationshipManagerType>("rm_type");

  // Set by MOOSE
  params.addPrivateParam<MooseMesh *>("mesh");
  params.registerBase("RelationshipManager");
  return params;
}

RelationshipManager::RelationshipManager(const InputParameters & parameters)
  : MooseObject(parameters),
    GhostingFunctor(),
    _mesh(*getCheckedPointerParam<MooseMesh *>(
        "mesh", "Mesh is null in GeometricRelationshipManager ctor")),
    _attach_geometric_early(getParam<bool>("attach_geometric_early")),
    _rm_type(getParam<Moose::RelationshipManagerType>("rm_type")),
    _cached_callbacks(Moose::RelationshipManagerType::Invalid),
    _has_set_remote_elem_removal_flag(false)
{
}

void
RelationshipManager::attachRelationshipManagers(Moose::RelationshipManagerType when_type)
{
  /**
   * If we cannot attach the geometric functor early, we have to prevent the mesh from deleting
   * elements that we might need for a future relationship manager.
   */
  if (!_has_set_remote_elem_removal_flag && !_attach_geometric_early && _mesh.isDistributedMesh())
  {
    _mesh.getMesh().allow_remote_element_removal(false);
    _has_set_remote_elem_removal_flag = true;
  }

  // Next make sure that we haven't already triggered this type of callback
  if ((_cached_callbacks & when_type) == when_type)
    return;

  /**
   * We have a few different cases to handle when attaching RelationshipManagers to the
   * corresponding libMesh objects.
   *
   * 1) GeometricRelationshipManager objects will only respond to the Geometric rm_type, However
   *    they can be attached either early or late.
   *
   * 2) AlgebraicRelationshipManager objects will respond to both the Geometric and Algebraic
   *    rm_types. Additionally, the object can decide to attach the geometric rm_type either early
   *    or late depending on the needs of the developer.
   *
   * Finally, we will make sure that each RelationshipManager receives only a single callback per
   * type.
   */
  auto obj_type = getType();
  if (obj_type == Moose::RelationshipManagerType::Geometric)
  {
    if ((_attach_geometric_early && when_type == Moose::RelationshipManagerType::Geometric) ||
        (!_attach_geometric_early && when_type == Moose::RelationshipManagerType::Algebraic))
    {
      attachRelationshipManagersInternal(obj_type);
      // Toggle the flag for this enum
      _cached_callbacks |= obj_type;
    }
  }
  else if (obj_type == Moose::RelationshipManagerType::Algebraic)
  {
    if (_attach_geometric_early && when_type == Moose::RelationshipManagerType::Geometric)
    {
      attachRelationshipManagersInternal(Moose::RelationshipManagerType::Geometric);
      _cached_callbacks |= Moose::RelationshipManagerType::Geometric;
    }
    else if (when_type == Moose::RelationshipManagerType::Algebraic)
    {
      // Note: We won't double add due to the early return above
      attachRelationshipManagersInternal(Moose::RelationshipManagerType::Geometric);
      attachRelationshipManagersInternal(Moose::RelationshipManagerType::Algebraic);

      _cached_callbacks |= Moose::RelationshipManagerType::Geometric;
      _cached_callbacks |= Moose::RelationshipManagerType::Algebraic;
    }
  }
}
