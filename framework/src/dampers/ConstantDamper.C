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

#include "ConstantDamper.h"

template<>
InputParameters validParams<ConstantDamper>()
{
  InputParameters params = validParams<Damper>();
  params.addRequiredParam<Real>("damping", "The percentage (between 0 and 1) of the newton update to take.");
  return params;
}

ConstantDamper::ConstantDamper(const std::string & name, InputParameters parameters) :
    Damper(name, parameters),
    _damping(getParam<Real>("damping"))
{
}

Real
ConstantDamper::computeQpDamping()
{
  return _damping;
}



