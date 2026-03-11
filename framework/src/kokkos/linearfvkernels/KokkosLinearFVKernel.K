//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosLinearFVKernel.h"

namespace Moose::Kokkos
{

InputParameters
LinearFVKernel::validParams()
{
  auto params = LinearSystemContributionObject::validParams();
  params += BlockRestrictable::validParams();
  params += NonADFunctorInterface::validParams();
  params += FVRelationshipManagerInterface::validParams();
  params.registerBase("LinearFVKernel");
  return params;
}

LinearFVKernel::LinearFVKernel(const InputParameters & parameters)
  : LinearSystemContributionObject(parameters),
    BlockRestrictable(this),
    NonADFunctorInterface(this),
    MooseVariableDependencyInterface(this),
    _var(libMesh::cast_ref<MooseLinearVariableFV<Real> &>(
        _subproblem.getVariable(_tid,
                                getParam<LinearVariableName>("variable"),
                                Moose::VarKindType::VAR_SOLVER,
                                Moose::VarFieldType::VAR_FIELD_STANDARD))),
    _var_num(_var.number()),
    _sys_num(_sys.number())
{
  addMooseVariableDependency(&_var);
  _kokkos_var.init(_var, Moose::SOLUTION_TAG);
}

LinearFVKernel::LinearFVKernel(const LinearFVKernel & object)
  : LinearSystemContributionObject(object),
    BlockRestrictable(object, {}),
    NonADFunctorInterface(object, {}),
    MooseVariableDependencyInterface(object, {}),
    _var(object._var),
    _kokkos_var(object._kokkos_var),
    _var_num(object._var_num),
    _sys_num(object._sys_num)
{
}

InputParameters
LinearFVElementalKernel::validParams()
{
  auto params = LinearFVKernel::validParams();
  params.registerSystemAttributeName("LinearFVElementalKernel");
  return params;
}

LinearFVElementalKernel::LinearFVElementalKernel(const InputParameters & parameters)
  : LinearFVKernel(parameters)
{
}

LinearFVElementalKernel::LinearFVElementalKernel(const LinearFVElementalKernel & object)
  : LinearFVKernel(object)
{
}

void
LinearFVElementalKernel::computeRightHandSide()
{
  Policy policy(0, numKokkosBlockElements());
  if (!_rhs_dispatcher)
    _rhs_dispatcher = DispatcherRegistry::build<RightHandSideLoop>(this, type());
  _rhs_dispatcher->parallelFor(policy);
}

void
LinearFVElementalKernel::computeMatrix()
{
  Policy policy(0, numKokkosBlockElements());
  if (!_matrix_dispatcher)
    _matrix_dispatcher = DispatcherRegistry::build<MatrixLoop>(this, type());
  _matrix_dispatcher->parallelFor(policy);
}

InputParameters
LinearFVFluxKernel::validParams()
{
  auto params = LinearFVKernel::validParams();
  params.addParam<bool>("force_boundary_execution",
                        false,
                        "Whether to force execution of this object on all external boundaries.");
  params.registerSystemAttributeName("LinearFVFluxKernel");
  return params;
}

LinearFVFluxKernel::LinearFVFluxKernel(const InputParameters & parameters)
  : LinearFVKernel(parameters),
    FaceArgProducerInterface(),
    _force_boundary_execution(getParam<bool>("force_boundary_execution"))
{
}

LinearFVFluxKernel::LinearFVFluxKernel(const LinearFVFluxKernel & object)
  : LinearFVKernel(object),
    FaceArgProducerInterface(),
    _force_boundary_execution(object._force_boundary_execution)
{
}

void
LinearFVFluxKernel::computeRightHandSide()
{
  Policy policy(0, numKokkosBlockSides());
  if (!_rhs_dispatcher)
    _rhs_dispatcher = DispatcherRegistry::build<RightHandSideLoop>(this, type());
  _rhs_dispatcher->parallelFor(policy);
}

void
LinearFVFluxKernel::computeMatrix()
{
  Policy policy(0, numKokkosBlockSides());
  if (!_matrix_dispatcher)
    _matrix_dispatcher = DispatcherRegistry::build<MatrixLoop>(this, type());
  _matrix_dispatcher->parallelFor(policy);
}

bool
LinearFVFluxKernel::hasFaceSide(const FaceInfo & fi, const bool fi_elem_side) const
{
  const auto ft = fi.faceType(std::make_pair(_var_num, _sys_num));
  if (fi_elem_side)
    return ft == FaceInfo::VarFaceNeighbors::ELEM || ft == FaceInfo::VarFaceNeighbors::BOTH;
  else
    return ft == FaceInfo::VarFaceNeighbors::NEIGHBOR || ft == FaceInfo::VarFaceNeighbors::BOTH;
}

} // namespace Moose::Kokkos
