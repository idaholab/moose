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
#include "FEBoundaryBaseUserObject.h"

template <>
InputParameters
validParams<FEBoundaryBaseUserObject>()
{
  InputParameters params = validParams<SideIntegralVariableUserObject>();

  params += validParams<FEIntegralBaseUserObjectParameters>();

  return params;
}

FEBoundaryBaseUserObject::FEBoundaryBaseUserObject(const InputParameters & parameters)
  : FEIntegralBaseUserObject(parameters)
{
  mooseInfo("Using FEInterface-type UserObject '",
            name(),
            "'.\nNote: it is your responsibility to ensure that the dimensionality, order, and "
            "series parameters for FunctionSeries '",
            _function_series.name(),
            "' are sane.");
}

Point
FEBoundaryBaseUserObject::getCentroid() const
{
  return _current_side_elem->centroid();
}

Real
FEBoundaryBaseUserObject::getVolume() const
{
  return _current_side_volume;
}

FEBoundaryBaseUserObject::~FEBoundaryBaseUserObject()
{
  // Nothing here
}
