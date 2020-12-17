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

defineLegacyParams(RelationshipManager);

InputParameters
RelationshipManager::validParams()
{
  InputParameters params = MooseObject::validParams();

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
   * This parameter is used to indicate which system and subsequent DofMap this relationship manager
   * should be applied to. This parameter is not meaningful when the RM is of geometric type only.
   * If this parameter is equal to ANY, then this RM can be applied to both non-linear and aux
   * systems.
   */
  params.addPrivateParam<Moose::RMSystemType>("system_type", Moose::RMSystemType::NONE);

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
    _moose_mesh(getCheckedPointerParam<MooseMesh *>(
        "mesh",
        "Mesh is null in RelationshipManager constructor. This could well be because No mesh file "
        "was supplied and no generation block was provided")),
    _dof_map(nullptr),
    _attach_geometric_early(getParam<bool>("attach_geometric_early")),
    _rm_type(getParam<Moose::RelationshipManagerType>("rm_type")),
    _use_displaced_mesh(getParam<bool>("use_displaced_mesh")),
    _system_type(getParam<Moose::RMSystemType>("system_type"))
{
  _for_whom.push_back(getParam<std::string>("for_whom"));
}

RelationshipManager::RelationshipManager(const RelationshipManager & other)
  : MooseObject(other._pars),
    GhostingFunctor(other),
    _moose_mesh(other._moose_mesh),
    _dof_map(other._dof_map),
    _attach_geometric_early(other._attach_geometric_early),
    _rm_type(other._rm_type),
    _for_whom(other._for_whom),
    _use_displaced_mesh(other._use_displaced_mesh),
    _system_type(other._system_type)
{
}

bool
RelationshipManager::isGeometric(Moose::RelationshipManagerType input_rm)
{
  return (input_rm & Moose::RelationshipManagerType::GEOMETRIC) ==
         Moose::RelationshipManagerType::GEOMETRIC;
}

bool
RelationshipManager::isAlgebraic(Moose::RelationshipManagerType input_rm)
{
  return (input_rm & Moose::RelationshipManagerType::ALGEBRAIC) ==
         Moose::RelationshipManagerType::ALGEBRAIC;
}

bool
RelationshipManager::isCoupling(Moose::RelationshipManagerType input_rm)
{
  return (input_rm & Moose::RelationshipManagerType::COUPLING) ==
         Moose::RelationshipManagerType::COUPLING;
}

bool
RelationshipManager::baseGreaterEqual(const RelationshipManager & rhs) const
{
  return isType(rhs._rm_type) && _system_type == rhs._system_type;
}

Moose::RelationshipManagerType RelationshipManager::geo_and_alg =
    Moose::RelationshipManagerType::GEOMETRIC | Moose::RelationshipManagerType::ALGEBRAIC;

Moose::RelationshipManagerType RelationshipManager::geo_alg_and_coupl =
    Moose::RelationshipManagerType::GEOMETRIC | Moose::RelationshipManagerType::ALGEBRAIC |
    Moose::RelationshipManagerType::COUPLING;

InputParameters
dummyParams()
{
  auto params = emptyInputParameters();
  params.set<std::string>("_moose_base") = "dummy";
  return params;
}

InputParameters
RelationshipManager::zeroLayerGhosting()
{
  auto params = dummyParams();
  params.addRelationshipManager("ElementSideNeighborLayers",
                                Moose::RelationshipManagerType::COUPLING,
                                [](const InputParameters &, InputParameters & rm_params) {
                                  rm_params.set<unsigned short>("layers") = 0;
                                });
  return params;
}

InputParameters
RelationshipManager::oneLayerGhosting(Moose::RelationshipManagerType rm_type)
{
  auto params = dummyParams();
  params.addRelationshipManager("ElementSideNeighborLayers", rm_type);
  return params;
}
