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
  InputParameters p = validParams<Kernel>();
  return p;
}


Diffusion::Diffusion(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters)
{

}

Diffusion::~Diffusion()
{

}

Real
Diffusion::computeQpResidual()
{
  return _grad_u[_qp] * _grad_test[_i][_qp];      
}

Real
Diffusion::computeQpJacobian()
{
  return _grad_phi[_j][_qp] * _grad_test[_i][_qp];
}
