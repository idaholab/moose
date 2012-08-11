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

#include "StandardMarker.h"

template<>
InputParameters validParams<StandardMarker>()
{
  InputParameters params = validParams<Marker>();
  params.addRequiredParam<VariableName>("indicator_field", "The name of the Indicator field that this Marker operates on.");
  return params;
}


StandardMarker::StandardMarker(const std::string & name, InputParameters parameters) :
    Marker(name, parameters)
{
}

int
StandardMarker::computeElementMarker()
{
  return 1;
}

