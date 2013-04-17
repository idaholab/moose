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

#include "AreaPostprocessor.h"

template<>
InputParameters validParams<AreaPostprocessor>()
{
InputParameters params = validParams<SideIntegralPostprocessor>();
  return params;
}

AreaPostprocessor::AreaPostprocessor(const std::string & name, InputParameters parameters) :
    SideIntegralPostprocessor(name, parameters)
{}

void
AreaPostprocessor::threadJoin(const UserObject &y)
{
  const AreaPostprocessor & pps = dynamic_cast<const AreaPostprocessor &>(y);
  _integral_value += pps._integral_value;
}

Real
AreaPostprocessor::computeQpIntegral()
{
  return 1.0;
}
