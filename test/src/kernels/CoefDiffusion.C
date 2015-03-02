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
#include "CoefDiffusion.h"

template<>
InputParameters validParams<CoefDiffusion>()
{
  InputParameters params = validParams<Kernel>();
  params.addCustomTypeParam("coef", 0.0, "CoefficientType", "The coefficient of diffusion");
  return params;
}

CoefDiffusion::CoefDiffusion(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _coef(getParam<Real>("coef"))
{
}

Real
CoefDiffusion::computeQpResidual()
{
  return _coef*_grad_test[_i][_qp]*_grad_u[_qp];
}

Real
CoefDiffusion::computeQpJacobian()
{
  return _coef*_grad_test[_i][_qp]*_grad_phi[_j][_qp];
}
