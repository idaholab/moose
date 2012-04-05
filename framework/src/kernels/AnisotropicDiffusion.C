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

#include "AnisotropicDiffusion.h"


template<>
InputParameters validParams<AnisotropicDiffusion>()
{
  InputParameters p = validParams<Kernel>();
  p.addRequiredParam<RealTensorValue>("tensor_coeff", "The Tensor to multiply the Diffusion operator by");
  return p;
}


AnisotropicDiffusion::AnisotropicDiffusion(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _k(getParam<RealTensorValue>("tensor_coeff"))
{
}

AnisotropicDiffusion::~AnisotropicDiffusion()
{
}

Real
AnisotropicDiffusion::computeQpResidual()
{
  return (_k * _grad_u[_qp]) * _grad_test[_i][_qp];
}

Real
AnisotropicDiffusion::computeQpJacobian()
{
  return (_k * _grad_phi[_j][_qp]) * _grad_test[_i][_qp];
}
