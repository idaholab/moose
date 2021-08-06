//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVKernel.h"
#include "Assembly.h"
#include "SubProblem.h"

void
FVKernel::setRMParams(const InputParameters & obj_params,
                      InputParameters & rm_params,
                      const unsigned short ghost_layers)
{
  rm_params.set<unsigned short>("layers") = ghost_layers;
  rm_params.set<bool>("use_point_neighbors") = obj_params.get<bool>("use_point_neighbors");
  rm_params.set<bool>("attach_geometric_early") = false;
  rm_params.set<bool>("use_displaced_mesh") = obj_params.get<bool>("use_displaced_mesh");
}

InputParameters
FVKernel::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += TransientInterface::validParams();
  params += BlockRestrictable::validParams();
  params += TaggingInterface::validParams();
  params += FunctorInterface::validParams();
  params.addRequiredParam<NonlinearVariableName>(
      "variable", "The name of the finite volume variable this kernel applies to");
  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the "
                        "displaced mesh for computation. Note that in "
                        "the case this is true but no displacements "
                        "are provided in the Mesh block the "
                        "undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  params.declareControllable("enable");

  params.addParam<unsigned short>("ghost_layers", 1, "The number of layers of elements to ghost.");
  params.addParam<bool>("use_point_neighbors",
                        false,
                        "Whether to use point neighbors, which introduces additional ghosting to "
                        "that used for simple face neighbors.");
  params.set<bool>("_residual_object") = true;

  // FV Kernels always need one layer of ghosting because when looping over
  // faces to compute fluxes, the elements on each side of the face may be on
  // different MPI ranks, but we still need to access them as a pair to
  // compute the numerical face flux.
  params.addRelationshipManager(
      "ElementSideNeighborLayers",
      Moose::RelationshipManagerType::GEOMETRIC | Moose::RelationshipManagerType::ALGEBRAIC |
          Moose::RelationshipManagerType::COUPLING,
      [](const InputParameters & obj_params, InputParameters & rm_params) {
        FVKernel::setRMParams(
            obj_params, rm_params, obj_params.get<unsigned short>("ghost_layers"));
      });

  params.registerBase("FVKernel");
  return params;
}

FVKernel::FVKernel(const InputParameters & params)
  : MooseObject(params),
    TaggingInterface(this),
    TransientInterface(this),
    BlockRestrictable(this),
    FunctionInterface(this),
    UserObjectInterface(this),
    PostprocessorInterface(this),
    SetupInterface(this),
    Restartable(this, "FVKernels"),
    FunctorInterface(this),
    _subproblem(*getCheckedPointerParam<SubProblem *>("_subproblem")),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _tid(params.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _mesh(_subproblem.mesh())
{
  _subproblem.haveADObjects(true);
  if (getParam<bool>("use_displaced_mesh"))
    paramError("use_displaced_mesh", "FV kernels do not yet support displaced mesh");
}
