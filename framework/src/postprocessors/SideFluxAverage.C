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

#include "SideFluxAverage.h"

template<>
InputParameters validParams<SideFluxAverage>()
{
  InputParameters params = validParams<SideFluxIntegral>();
  return params;
}

SideFluxAverage::SideFluxAverage(const std::string & name, InputParameters parameters) :
    SideFluxIntegral(name, parameters),
    _volume(0)
{}

void
SideFluxAverage::initialize()
{
  SideIntegralVariablePostprocessor::initialize();
  _volume = 0;
}

void
SideFluxAverage::execute()
{
  SideIntegralVariablePostprocessor::execute();
  _volume += _current_side_volume;
}

Real
SideFluxAverage::getValue()
{
  Real integral = SideIntegralVariablePostprocessor::getValue();

  gatherSum(_volume);

  return integral / _volume;
}

void
SideFluxAverage::threadJoin(const UserObject & y)
{
  SideIntegralVariablePostprocessor::threadJoin(y);
  const SideFluxAverage & pps = dynamic_cast<const SideFluxAverage &>(y);
  _volume += pps._volume;
}
