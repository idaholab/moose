//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MassFluxPenalty.h"

// MOOSE includes
#include "MooseVariableFE.h"

#include "libmesh/utility.h"

registerMooseObject("NavierStokesApp", MassFluxPenalty);

InputParameters
MassFluxPenalty::validParams()
{
  InputParameters params = ADDGKernel::validParams();
  params.addRequiredCoupledVar("u", "The x-velocity");
  params.addRequiredCoupledVar("v", "The y-velocity");
  params.addRequiredParam<unsigned short>("component",
                                          "The velocity component this object is being applied to");
  params.addParam<Real>("gamma", 1, "The penalty to multiply the jump");
  params.addClassDescription(
      "introduces a jump correction for grad-div stabilization for discontinuous Galerkin methods");
  return params;
}

MassFluxPenalty::MassFluxPenalty(const InputParameters & parameters)
  : ADDGKernel(parameters),
    _vel_x(adCoupledValue("u")),
    _vel_x_neighbor(adCoupledNeighborValue("u")),
    _vel_y(adCoupledValue("v")),
    _vel_y_neighbor(adCoupledNeighborValue("v")),
    _comp(getParam<unsigned short>("component")),
    _matrix_only(getParam<bool>("matrix_only")),
    _gamma(getParam<Real>("gamma"))
{
  if (_mesh.dimension() > 2)
    mooseError("This class only supports 2D simulations at this time");
}

void
MassFluxPenalty::computeResidual()
{
  if (!_matrix_only)
    ADDGKernel::computeResidual();
}

ADReal
MassFluxPenalty::computeQpResidual(Moose::DGResidualType type)
{
  ADReal r = 0.0;
  const ADRealVectorValue soln_jump(
      _vel_x[_qp] - _vel_x_neighbor[_qp], _vel_y[_qp] - _vel_y_neighbor[_qp], 0);

  switch (type)
  {
    case Moose::Element:
      r = _gamma * soln_jump * _normals[_qp] * _test[_i][_qp] * _normals[_qp](_comp);
      break;

    case Moose::Neighbor:
      r = -_gamma * soln_jump * _normals[_qp] * _test_neighbor[_i][_qp] * _normals[_qp](_comp);
      break;
  }

  return r;
}
