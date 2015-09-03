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

#include "ADDiffusion.h"


template<>
InputParameters validParams<ADDiffusion>()
{
  InputParameters p = validParams<ADKernel>();
  return p;
}

ADDiffusion::ADDiffusion(const InputParameters & parameters) :
    ADKernel(parameters)
{
}

ADReal
ADDiffusion::computeQpResidual()
{
  return _grad_u[_qp] * _grad_test[_i][_qp];
}
