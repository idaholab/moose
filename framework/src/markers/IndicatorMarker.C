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

#include "IndicatorMarker.h"

template<>
InputParameters validParams<IndicatorMarker>()
{
  InputParameters params = validParams<Marker>();
  params.addRequiredParam<IndicatorName>("indicator", "The name of the Indicator that this Marker uses.");
  return params;
}


IndicatorMarker::IndicatorMarker(const std::string & name, InputParameters parameters) :
    Marker(name, parameters),
    _error_vector(getErrorVector(parameters.get<IndicatorName>("indicator")))
{
}
