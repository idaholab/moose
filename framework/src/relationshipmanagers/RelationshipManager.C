//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RelationshipManager.h"
#include "MooseApp.h"

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

  /**
   * The name of the object (or Action) requesting this RM
   */
  params.addRequiredParam<std::string>("for_whom", "What object is requesting this RM?");

  params.addParam<bool>(
      "use_displaced_mesh",
      false,
      "Whether this RM should be placed on the undisplaced or displaced problem. Note: yes, it "
      "still says 'mesh' to match with common parameter name in MOOSE - but if it's purely "
      "algebraic then it's going to the DofMap no matter what!");

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
    _use_displaced_mesh(getParam<bool>("use_displaced_mesh"))
{
  _for_whom.push_back(getParam<std::string>("for_whom"));
}
