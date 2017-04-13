/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "INSDivergenceAux.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<INSDivergenceAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addClassDescription("Computes h_min / |u|.");
  // Coupled variables
  params.addRequiredCoupledVar("u", "x-velocity");
  params.addCoupledVar("v", "y-velocity"); // only required in 2D and 3D
  params.addCoupledVar("w", "z-velocity"); // only required in 3D

  return params;
}

INSDivergenceAux::INSDivergenceAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _grad_u_vel(coupledGradient("u")),
    _grad_v_vel(_mesh.dimension() >= 2 ? coupledGradient("v") : _grad_zero),
    _grad_w_vel(_mesh.dimension() == 3 ? coupledGradient("w") : _grad_zero)
{
}

Real
INSDivergenceAux::computeValue()
{
  // div U = du/dx + dv/dy + dw/dz
  return _grad_u_vel[_qp](0) + _grad_v_vel[_qp](1) + _grad_w_vel[_qp](2);
}
