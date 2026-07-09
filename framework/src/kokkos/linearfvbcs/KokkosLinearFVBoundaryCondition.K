//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosLinearFVBoundaryCondition.h"

#include "FEProblemBase.h"
#include "InputParameters.h"
#include "SubProblem.h"
#include "SystemBase.h"

namespace Moose::Kokkos
{

InputParameters
LinearFVBoundaryCondition::validParams()
{
  auto params = MooseObject::validParams();
  params += TransientInterface::validParams();
  params += MeshChangedInterface::validParams();
  params += BoundaryRestrictableRequired::validParams();

  params.addPrivateParam<bool>(MooseBase::kokkos_object_param, true);
  params.addRequiredParam<LinearVariableName>(
      "variable", "The name of the variable this boundary condition supplies data for");
  params.declareControllable("enable");
  params.set<bool>("_residual_object") = false;
  params.registerBase("LinearFVBoundaryCondition");
  params.registerSystemAttributeName("LinearFVBoundaryCondition");
  return params;
}

LinearFVBoundaryCondition::LinearFVBoundaryCondition(const InputParameters & parameters)
  : MooseObject(parameters),
    SetupInterface(this),
    FunctionInterface(this),
    UserObjectInterface(this),
    TransientInterface(this),
    PostprocessorInterface(this),
    VectorPostprocessorInterface(this, false),
    Restartable(this, getBase() + "s"),
    MeshChangedInterface(parameters),
    BoundaryRestrictableRequired(this, false),
    MeshHolder(*getCheckedPointerParam<SubProblem *>("_subproblem")->mesh().getKokkosMesh()),
    SystemHolder(getCheckedPointerParam<SystemBase *>("_sys")->feProblem().getKokkosSystems()),
    _fe_problem(*getParam<FEProblemBase *>("_fe_problem_base")),
    _t(TransientInterface::_t),
    _t_old(TransientInterface::_t_old),
    _t_step(TransientInterface::_t_step),
    _dt(TransientInterface::_dt),
    _dt_old(TransientInterface::_dt_old)
{
  const auto * const subproblem = getCheckedPointerParam<SubProblem *>("_subproblem");
  _var.init(subproblem->getVariable(0,
                                    getParam<LinearVariableName>("variable"),
                                    Moose::VarKindType::VAR_SOLVER,
                                    Moose::VarFieldType::VAR_FIELD_STANDARD),
            Moose::SOLUTION_TAG);
}

LinearFVBoundaryCondition::LinearFVBoundaryCondition(const LinearFVBoundaryCondition & object)
  : MooseObject(object, {}),
    SetupInterface(object, {}),
    FunctionInterface(object, {}),
    UserObjectInterface(object, {}),
    TransientInterface(object, {}),
    PostprocessorInterface(object, {}),
    VectorPostprocessorInterface(object, {}),
    Restartable(object, {}),
    MeshChangedInterface(object, {}),
    BoundaryRestrictableRequired(object, {}),
    MeshHolder(object),
    SystemHolder(object),
    _fe_problem(object._fe_problem),
    _var(object._var),
    _boundary_data(object._boundary_data),
    _t(object._t),
    _t_old(object._t_old),
    _t_step(object._t_step),
    _dt(object._dt),
    _dt_old(object._dt_old)
{
}

void
LinearFVBoundaryCondition::initialSetup()
{
  const auto n_elems =
      libMesh::cast_int<MOOSE_KOKKOS_INDEX_TYPE>(kokkosMesh().getNumLocalElements());
  const auto max_sides = libMesh::cast_int<MOOSE_KOKKOS_INDEX_TYPE>(
      getCheckedPointerParam<SubProblem *>("_subproblem")->mesh().getMaxSidesPerElem());

  _boundary_data.value.create(max_sides, n_elems);
  _boundary_data.normal_gradient.create(max_sides, n_elems);
  _boundary_data.value = BoundaryRelation{};
  _boundary_data.normal_gradient = BoundaryRelation{};
}

bool
LinearFVBoundaryCondition::hasBoundaryValue() const
{
  return DispatcherRegistry::hasUserMethod<BoundaryValueLoop>(type());
}

bool
LinearFVBoundaryCondition::hasBoundaryNormalGradient() const
{
  return DispatcherRegistry::hasUserMethod<BoundaryNormalGradientLoop>(type());
}

void
LinearFVBoundaryCondition::computeBoundaryValueData()
{
  if (!hasBoundaryValue())
    mooseError("Kokkos linear FV boundary condition '",
               name(),
               "' (type '",
               type(),
               "') was asked to compute boundary values, but it does not override "
               "computeBoundaryValue().");

  Policy policy(0, numKokkosBoundarySides());
  if (!_boundary_value_dispatcher)
    _boundary_value_dispatcher = DispatcherRegistry::build<BoundaryValueLoop>(this, type());
  _boundary_value_dispatcher->parallelFor(policy);
}

void
LinearFVBoundaryCondition::computeBoundaryNormalGradientData()
{
  if (!hasBoundaryNormalGradient())
    mooseError("Kokkos linear FV boundary condition '",
               name(),
               "' (type '",
               type(),
               "') was asked to compute boundary normal gradients, but it does not override "
               "computeBoundaryNormalGradient().");

  Policy policy(0, numKokkosBoundarySides());
  if (!_boundary_normal_gradient_dispatcher)
    _boundary_normal_gradient_dispatcher =
        DispatcherRegistry::build<BoundaryNormalGradientLoop>(this, type());
  _boundary_normal_gradient_dispatcher->parallelFor(policy);
}

} // namespace Moose::Kokkos
