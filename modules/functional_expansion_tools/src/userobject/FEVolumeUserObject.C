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

// Module includes
#include "FEVolumeUserObject.h"

template <>
InputParameters
validParams<FEVolumeUserObject>()
{
  InputParameters params = validParams<ElementIntegralVariableUserObject>();

  params += validParams<FEIntegralBaseUserObjectParameters>();

  params.addClassDescription("Generates an FE representation of a variable value over"
                             " a volume using a 'FunctionSeries'-type Function");

  return params;
}

FEVolumeUserObject::FEVolumeUserObject(const InputParameters & parameters)
  : FEIntegralBaseUserObject<ElementIntegralVariableUserObject>(parameters)
{
  mooseInfo("Using FEVolumeUserObject '",
            name(),
            "'.\nNote: it is your responsibility to ensure that the dimensionality, order, and "
            "series parameters for FunctionSeries '",
            _function_series.name(),
            "' are sane.");
}

Point
FEVolumeUserObject::getCentroid() const
{
  return _current_elem->centroid();
}

Real
FEVolumeUserObject::getVolume() const
{
  return _current_elem_volume;
}

FEVolumeUserObject::~FEVolumeUserObject()
{
  // Nothing here
}
