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
    NonADFunctorInterface(this)
{
}

LinearFVBoundaryCondition::LinearFVBoundaryCondition(const LinearFVBoundaryCondition & object)
  : LinearSystemContributionObject(object),
    BoundaryRestrictableRequired(object, {}),
    GeometricSearchInterface(object, {}),
    NonADFunctorInterface(object, {}),
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

} // namespace Moose::Kokkos
