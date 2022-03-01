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
    _attach_geometric_early(getParam<bool>("attach_geometric_early")),
    _rm_type(getParam<Moose::RelationshipManagerType>("rm_type")),
    _use_displaced_mesh(getParam<bool>("use_displaced_mesh"))
{
  _for_whom.push_back(getParam<std::string>("for_whom"));
}

RelationshipManager::RelationshipManager(const RelationshipManager & other)
  : MooseObject(other._pars),
    GhostingFunctor(other),
    _moose_mesh(other._moose_mesh),
    _attach_geometric_early(other._attach_geometric_early),
    _rm_type(other._rm_type),
    _for_whom(other._for_whom),
    _use_displaced_mesh(other._use_displaced_mesh)
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
  // !attach early = attach late
  // If we are supposed to be attached late, then we should take precedence over the rhs. You can
  // think of this as being >= because we if we are attaching late, then we are asking that *all*
  // elements be geometrically ghosted during the initial mesh preparation phase. We will only allow
  // remote elements to be deleted later on after addition of late geomeric ghosting functors (at
  // the same time as algebraic and coupling)
  return isType(rhs._rm_type) && (!_attach_geometric_early >= !rhs._attach_geometric_early);
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
                                [](const InputParameters &, InputParameters & rm_params)
                                { rm_params.set<unsigned short>("layers") = 0; });
  return params;
}

InputParameters
RelationshipManager::oneLayerGhosting(Moose::RelationshipManagerType rm_type)
{
  auto params = dummyParams();
  params.addRelationshipManager("ElementSideNeighborLayers", rm_type);
  return params;
}

void
RelationshipManager::init(const MeshBase & mesh, const DofMap * const dof_map)
{
  mooseAssert(_dof_map ? dof_map == _dof_map : true,
              "Trying to initialize with a different dof map");

  _dof_map = dof_map;

  // It is conceivable that this init method gets called twice, once during early geometric setup
  // and later when we're doing algebraic/coupling (and late geometric) setup. There might be new
  // information available during the latter call that the derived RelationshipManager can
  // leverage, so we should make sure we call through to internal init both times
  internalInitWithMesh(mesh);

  // One would hope that internalInitWithMesh set the mesh, but we can't be sure
  set_mesh(&mesh);

  _inited = true;
}
