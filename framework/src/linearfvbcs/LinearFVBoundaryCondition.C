//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVBoundaryCondition.h"
#include "Problem.h"
#include "SystemBase.h"
#include "MooseVariableFV.h"

namespace
{
SystemBase &
changeSystem(const InputParameters & params_in, MooseVariableBase & var)
{
  SystemBase & var_sys = var.sys();
  auto & params = const_cast<InputParameters &>(params_in);
  params.set<SystemBase *>("_sys") = &var_sys;
  return var_sys;
}
}

InputParameters
LinearFVBoundaryCondition::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += TransientInterface::validParams();
  params += BoundaryRestrictableRequired::validParams();
  params += TaggingInterface::validParams();
  params += ADFunctorInterface::validParams();

  params.addRequiredParam<NonlinearVariableName>(
      "variable", "The name of the variable that this boundary condition applies to");
  params.declareControllable("enable");
  params.registerBase("LinearFVBoundaryCondition");
  params.registerSystemAttributeName("LinearFVBoundaryCondition");
  return params;
}

LinearFVBoundaryCondition::LinearFVBoundaryCondition(const InputParameters & parameters)
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
    MooseVariableDependencyInterface(this),
    ADFunctorInterface(this),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _subproblem(*getCheckedPointerParam<SubProblem *>("_subproblem")),
    _mesh(_subproblem.mesh()),
    _fv_problem(*getCheckedPointerParam<FVProblemBase *>("_fe_problem_base")),
    _var(dynamic_cast<MooseLinearVariableFV<Real> *>(
        &_fv_problem.getVariable(_tid,
                                 parameters.varName("variable", name()),
                                 Moose::VarKindType::VAR_LINEAR,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD))),
    _sys(changeSystem(parameters, *_var))
{
  if (!_var)
    paramError("variable",
               "The variable defined for boundary condition ",
               name(),
               " is not derived from MooseLinearVariableFV!");

  addMooseVariableDependency(_var);
}
