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

#include "LaplacianJumpIndicator.h"

template<>
InputParameters validParams<LaplacianJumpIndicator>()
{
  InputParameters params = validParams<JumpIndicator>();
  return params;
}


LaplacianJumpIndicator::LaplacianJumpIndicator(const std::string & name, InputParameters parameters) :
    JumpIndicator(name, parameters),
    _second_u(second()),
    _second_u_neighbor(neighborSecond())
{
  _mesh.errorIfParallelDistribution("LaplacianJumpIndicator");
}


Real
LaplacianJumpIndicator::computeQpIntegral()
{
  Real jump = (_second_u[_qp].tr() - _second_u_neighbor[_qp].tr());

  return jump*jump;
}

