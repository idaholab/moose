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
#include "InterfaceOrientationMaterial.h"

template<>
InputParameters validParams<InterfaceOrientationMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("c", "variable");
  return params;
}

InterfaceOrientationMaterial::InterfaceOrientationMaterial(const InputParameters & parameters) :
    Material(parameters),
    _eps(declareProperty<Real>("eps")),
    _deps(declareProperty<Real>("eps1")),
    _u(coupledValue("c")),
    _grad_u(coupledGradient("c"))
{
  // this currently only works in 2D simulations
  if (_mesh.dimension() != 2)
    mooseError("InterfaceOrientationMaterial requires a two-dimensional mesh.");
}

void
InterfaceOrientationMaterial::computeQpProperties()
{
  // cosine of the gradient orientation angle
  Real cos_grad;
  const Real grad2 = _grad_u[_qp] * _grad_u[_qp];
  if (grad2 == 0)
    cos_grad = 0;
  else
    cos_grad = _grad_u[_qp](0) / std::sqrt(grad2);

  const Real angle = std::acos(cos_grad);

  _eps[_qp]= 0.01 * (1.0 + 0.04 * std::cos(6 * (angle - libMesh::pi/2.0)));
  _deps[_qp]= -0.01 * 0.04 * 6.0 * std::sin(6 * (angle - libMesh::pi/2.0));
}
