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

#include "MaxIncrement.h"

template <>
InputParameters
validParams<MaxIncrement>()
{
  InputParameters params = validParams<ElementDamper>();
  params.addRequiredParam<Real>("max_increment", "The maximum newton increment for the variable.");
  return params;
}

MaxIncrement::MaxIncrement(const InputParameters & parameters)
  : ElementDamper(parameters), _max_increment(parameters.get<Real>("max_increment"))
{
}

Real
MaxIncrement::computeQpDamping()
{

  if (std::abs(_u_increment[_qp]) > _max_increment)
  {
    return std::abs(_max_increment / _u_increment[_qp]);
  }

  return 1.0;
}
