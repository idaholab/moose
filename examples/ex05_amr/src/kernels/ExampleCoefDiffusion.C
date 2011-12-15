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

#include "ExampleCoefDiffusion.h"

template<>
InputParameters validParams<ExampleCoefDiffusion>()
{
  InputParameters params = validParams<Kernel>();
  params.set<Real>("coef")=0.0;
  return params;
}

ExampleCoefDiffusion::ExampleCoefDiffusion(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
   _coef(getParam<Real>("coef"))
{}

Real
ExampleCoefDiffusion::computeQpResidual()
{
  return _coef*_grad_test[_i][_qp]*_grad_u[_qp];
}

Real
ExampleCoefDiffusion::computeQpJacobian()
{
  return _coef*_grad_test[_i][_qp]*_grad_phi[_j][_qp];
}
