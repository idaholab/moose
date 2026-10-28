//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestMortarContactPressureProvider.h"
#include "MooseVariableFE.h"

registerMooseObject("MooseTestApp", TestMortarContactPressureProvider);

InputParameters
TestMortarContactPressureProvider::validParams()
{
  InputParameters params = MortarUserObject::validParams();
  params.addClassDescription(
      "Provides the physical contact pressure from a nodal variable holding a scaled pressure.");
  params.addRequiredCoupledVar("scaled_pressure",
                               "The nodal variable holding the scaled contact pressure.");
  params.addRequiredRangeCheckedParam<Real>(
      "scaling", "scaling != 0", "The factor by which the stored pressure is scaled.");
  return params;
}

TestMortarContactPressureProvider::TestMortarContactPressureProvider(
    const InputParameters & parameters)
  : MortarUserObject(parameters),
    MortarContactPressureProvider(),
    _var(*getVar("scaled_pressure", 0)),
    _scaling(getParam<Real>("scaling"))
{
  if (!_var.isNodal())
    paramError("scaled_pressure", "The scaled pressure variable must be nodal.");
}

ADReal
TestMortarContactPressureProvider::nodalContactPressure(const Node & node) const
{
  return _var.getNodalValue(node) / _scaling;
}

const VariableTestValue &
TestMortarContactPressureProvider::tractionBasis() const
{
  return _var.phiLower();
}
