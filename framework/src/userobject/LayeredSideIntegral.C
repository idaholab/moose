//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LayeredSideIntegral.h"

template <>
InputParameters
validParams<LayeredSideIntegral>()
{
  InputParameters params = validParams<SideIntegralVariableUserObject>();
  params += validParams<LayeredBase>();
  return params;
}

LayeredSideIntegral::LayeredSideIntegral(const InputParameters & parameters)
  : SideIntegralVariableUserObject(parameters), LayeredBase(parameters)
{
}

void
LayeredSideIntegral::initialize()
{
  SideIntegralVariableUserObject::initialize();
  LayeredBase::initialize();
}

void
LayeredSideIntegral::execute()
{
  Real integral_value = computeIntegral();

  unsigned int layer = getLayer(_current_elem->centroid());

  setLayerValue(layer, getLayerValue(layer) + integral_value);
}

void
LayeredSideIntegral::finalize()
{
  LayeredBase::finalize();
}

void
LayeredSideIntegral::threadJoin(const UserObject & y)
{
  SideIntegralVariableUserObject::threadJoin(y);
  LayeredBase::threadJoin(y);
}
