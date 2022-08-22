//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiscreteLineSegmentInterfaceTestAux.h"

registerMooseObject("ThermalHydraulicsTestApp", DiscreteLineSegmentInterfaceTestAux);

InputParameters
DiscreteLineSegmentInterfaceTestAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params += DiscreteLineSegmentInterface::validParams();

  params.addClassDescription("Tests DiscreteLineSegmentInterface.");

  return params;
}

DiscreteLineSegmentInterfaceTestAux::DiscreteLineSegmentInterfaceTestAux(const InputParameters & params)
  : AuxKernel(params), DiscreteLineSegmentInterface(this)
{
}

Real
DiscreteLineSegmentInterfaceTestAux::computeValue()
{
  const Point p = isNodal() ? *_current_node : _q_point[_qp];
  return computeAxialCoordinate(p);
}
