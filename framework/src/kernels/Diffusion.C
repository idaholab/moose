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

#include "Diffusion.h"

template<>
InputParameters validParams<Diffusion>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Implements the Diffusion operator (gradient of 'u')");
  return params;
}


Diffusion::Diffusion(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters)
{}

Real
Diffusion::computeQpResidual()
{
  return _grad_test[_i][_qp]*_grad_u[_qp];
}

Real
Diffusion::computeQpJacobian()
{
  return _grad_test[_i][_qp]*_grad_phi[_j][_qp];
}
