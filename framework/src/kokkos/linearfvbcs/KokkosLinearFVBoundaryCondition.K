//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosLinearFVBoundaryCondition.h"

namespace Moose::Kokkos
{

InputParameters
LinearFVBoundaryCondition::validParams()
{
  auto params = LinearSystemContributionObject::validParams();
  params += BoundaryRestrictableRequired::validParams();
  params += GeometricSearchInterface::validParams();
  params += NonADFunctorInterface::validParams();
  params.registerBase("LinearFVBoundaryCondition");
  params.registerSystemAttributeName("LinearFVBoundaryCondition");
  return params;
}

LinearFVBoundaryCondition::LinearFVBoundaryCondition(const InputParameters & parameters)
  : LinearSystemContributionObject(parameters),
    BoundaryRestrictableRequired(this, false),
    GeometricSearchInterface(this),
    NonADFunctorInterface(this),
    FaceArgProducerInterface(),
    _moose_var(libMesh::cast_ref<MooseLinearVariableFV<Real> &>(
        _subproblem.getVariable(0,
                                getParam<LinearVariableName>("variable"),
                                Moose::VarKindType::VAR_SOLVER,
                                Moose::VarFieldType::VAR_FIELD_STANDARD)))
{
  _var.init(_moose_var, Moose::SOLUTION_TAG);
}

LinearFVBoundaryCondition::LinearFVBoundaryCondition(const LinearFVBoundaryCondition & object)
  : LinearSystemContributionObject(object),
    BoundaryRestrictableRequired(object, {}),
    GeometricSearchInterface(object, {}),
    NonADFunctorInterface(object, {}),
    FaceArgProducerInterface(),
    _moose_var(object._moose_var),
    _var(object._var),
    _bc_data_matrix(object._bc_data_matrix),
    _bc_data_rhs(object._bc_data_rhs)
{
}

void
LinearFVBoundaryCondition::initBCData(const LinearFVFluxKernel::BoundaryFaceData & data)
{
  _bc_data_matrix = data.matrix_coeff;
  _bc_data_rhs = data.rhs_coeff;
}

void
LinearFVBoundaryCondition::computeRightHandSide()
{
  Policy policy(0, numKokkosBoundarySides());
  if (!_rhs_dispatcher)
    _rhs_dispatcher = DispatcherRegistry::build<RightHandSideLoop>(this, type());
  _rhs_dispatcher->parallelFor(policy);
}

void
LinearFVBoundaryCondition::computeMatrix()
{
  Policy policy(0, numKokkosBoundarySides());
  if (!_matrix_dispatcher)
    _matrix_dispatcher = DispatcherRegistry::build<MatrixLoop>(this, type());
  _matrix_dispatcher->parallelFor(policy);
}

bool
LinearFVBoundaryCondition::hasFaceSide(const FaceInfo & fi, const bool fi_elem_side) const
{
  const auto ft = fi.faceType(std::make_pair(_var.var(), _var.sys()));
  if (fi_elem_side)
    return ft == FaceInfo::VarFaceNeighbors::ELEM || ft == FaceInfo::VarFaceNeighbors::BOTH;
  else
    return ft == FaceInfo::VarFaceNeighbors::NEIGHBOR || ft == FaceInfo::VarFaceNeighbors::BOTH;
}

} // namespace Moose::Kokkos
