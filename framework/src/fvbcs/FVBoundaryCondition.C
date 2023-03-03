//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVBoundaryCondition.h"
#include "Problem.h"
#include "SystemBase.h"
#include "MooseVariableFV.h"

namespace
{
SystemBase &
changeSystem(const InputParameters & params_in, MooseVariableFV<Real> & fv_var)
{
  SystemBase & var_sys = fv_var.sys();
  auto & params = const_cast<InputParameters &>(params_in);
  params.set<SystemBase *>("_sys") = &var_sys;
  return var_sys;
}
}

InputParameters
FVBoundaryCondition::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += TransientInterface::validParams();
  params += BoundaryRestrictableRequired::validParams();
  params += TaggingInterface::validParams();
  params += ADFunctorInterface::validParams();

  params.addRequiredParam<NonlinearVariableName>(
      "variable", "The name of the variable that this boundary condition applies to");
  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the "
                        "displaced mesh for computation.  Note that "
                        "in the case this is true but no "
                        "displacements are provided in the Mesh block "
                        "the undisplaced mesh will still be used.");

  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");
  params.addCoupledVar("displacements", "The displacements");
  params.declareControllable("enable");
  params.registerBase("FVBoundaryCondition");
  return params;
}

FVBoundaryCondition::FVBoundaryCondition(const InputParameters & parameters)
  : MooseObject(parameters),
    BoundaryRestrictableRequired(this, false),
    SetupInterface(this),
    FunctionInterface(this),
    DistributionInterface(this),
    UserObjectInterface(this),
    TransientInterface(this),
    PostprocessorInterface(this),
    VectorPostprocessorInterface(this),
    GeometricSearchInterface(this),
    MeshChangedInterface(parameters),
    TaggingInterface(this),
    MooseVariableInterface<Real>(this,
                                 false,
                                 "variable",
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    MooseVariableDependencyInterface(this),
    ADFunctorInterface(this),
    _var(*mooseVariableFV()),
    _subproblem(*getCheckedPointerParam<SubProblem *>("_subproblem")),
    _fv_problem(*getCheckedPointerParam<FVProblemBase *>("_fe_problem_base")),
    _sys(changeSystem(parameters, _var)),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid, _var.kind() == Moose::VAR_NONLINEAR ? _sys.number() : 0)),
    _mesh(_subproblem.mesh())
{
  _subproblem.haveADObjects(true);
  addMooseVariableDependency(&_var);

  if (getParam<bool>("use_displaced_mesh"))
    paramError("use_displaced_mesh", "FV boundary conditions do not yet support displaced mesh");
}

Moose::FaceArg
FVBoundaryCondition::singleSidedFaceArg(const FaceInfo * fi,
                                        const Moose::FV::LimiterType limiter_type,
                                        const bool correct_skewness) const
{
  if (!fi)
    fi = _face_info;

  return makeFace(*fi, limiter_type, true, correct_skewness);
}

bool
FVBoundaryCondition::hasFaceSide(const FaceInfo & fi, bool fi_elem_side) const
{
  const auto ft = fi.faceType(_var.name());
  if (fi_elem_side)
    return ft == FaceInfo::VarFaceNeighbors::ELEM || ft == FaceInfo::VarFaceNeighbors::BOTH;
  else
    return ft == FaceInfo::VarFaceNeighbors::NEIGHBOR || ft == FaceInfo::VarFaceNeighbors::BOTH;
}
