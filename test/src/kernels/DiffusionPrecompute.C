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
#include "DiffusionPrecompute.h"

template <>
InputParameters
validParams<DiffusionPrecompute>()
{
  InputParameters p = validParams<KernelGrad>();
  return p;
}

DiffusionPrecompute::DiffusionPrecompute(const InputParameters & parameters)
  : KernelGrad(parameters)
{
}

DiffusionPrecompute::~DiffusionPrecompute() {}

RealGradient
DiffusionPrecompute::precomputeQpResidual()
{
  // Note we do not multiple by the gradient of the test function.  That is done in the parent class
  return _grad_u[_qp];
}

RealGradient
DiffusionPrecompute::precomputeQpJacobian()
{
  // Note we do not multiple by the gradient of the test function.  That is done in the parent class
  return _grad_phi[_j][_qp];
}
