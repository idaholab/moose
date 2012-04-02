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

#include "MixedValue2BC.h"

template<>
InputParameters validParams<MixedValue2BC>()
{
  InputParameters params = validParams<IntegratedBC>();
  return params;
}

MixedValue2BC::MixedValue2BC(const std::string & name, InputParameters parameters) :
  IntegratedBC(name, parameters)
{
}

Real
MixedValue2BC::computeQpResidual()
{
  return -_test[_i][_qp]*_q_point[_qp](0);
//  return (_grad_u[_qp])(1)*_test[_i][_qp] - _q_point[_qp](0);
}

