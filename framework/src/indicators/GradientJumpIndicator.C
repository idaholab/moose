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

#include "GradientJumpIndicator.h"

template<>
InputParameters validParams<GradientJumpIndicator>()
{
  InputParameters params = validParams<JumpIndicator>();
  return params;
}


GradientJumpIndicator::GradientJumpIndicator(const InputParameters & parameters) :
    JumpIndicator(parameters)
{
}


Real
GradientJumpIndicator::computeQpIntegral()
{
  Real jump = (_grad_u[_qp] - _grad_u_neighbor[_qp])*_normals[_qp];

  return jump*jump;
}


