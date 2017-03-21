/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// MOOSE includes
#include "VectorMagnitudeAux.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<VectorMagnitudeAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("x", "x-component of the vector");
  params.addCoupledVar("y", "y-component of the vector");
  params.addCoupledVar("z", "z-component of the vector");

  return params;
}

VectorMagnitudeAux::VectorMagnitudeAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _x(coupledValue("x")),
    _y(_mesh.dimension() >= 2 ? coupledValue("y") : _zero),
    _z(_mesh.dimension() >= 3 ? coupledValue("z") : _zero)
{
}

Real
VectorMagnitudeAux::computeValue()
{
  return std::sqrt((_x[_qp] * _x[_qp]) + (_y[_qp] * _y[_qp]) + (_z[_qp] * _z[_qp]));
}
