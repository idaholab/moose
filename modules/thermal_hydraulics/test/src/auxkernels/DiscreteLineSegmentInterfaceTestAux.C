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

  MooseEnum test_type("axial_coord radial_coord axial_section_index axial_element_index");
  params.addRequiredParam<MooseEnum>("test_type", test_type, "Type of test");

  params.addClassDescription("Tests DiscreteLineSegmentInterface.");

  return params;
}

DiscreteLineSegmentInterfaceTestAux::DiscreteLineSegmentInterfaceTestAux(const InputParameters & params)
  : AuxKernel(params), DiscreteLineSegmentInterface(this), _test_type(getParam<MooseEnum>("test_type"))
{
}

Real
DiscreteLineSegmentInterfaceTestAux::computeValue()
{
  const Point p = isNodal() ? *_current_node : _q_point[_qp];

  if (_test_type == "axial_coord")
    return computeAxialCoordinate(p);
  else if (_test_type == "radial_coord")
    return computeRadialCoordinate(p);
  else if (_test_type == "axial_section_index")
    return getAxialSectionIndex(p);
  else if (_test_type == "axial_element_index")
    return getAxialElementIndex(p);
  else
    mooseError("Invalid 'test_type'.");
}
