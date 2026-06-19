//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NS.h"
#include "INSPressureGradientBC.h"

registerMooseObject("NavierStokesApp", INSPressureGradientBC);

InputParameters
INSPressureGradientBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();

  params.addClassDescription(
      "Adds implicit pressure term on a boundary. This is appropriate on wall and inlet boundaries "
      "when the pressure term is integrated by parts");
  params.addRequiredCoupledVar(NS::pressure, "The current value of the pressure");
  params.addRequiredParam<unsigned>(
      "component", "(0,1,2) = (x,y,z) for which momentum component this BC is applied to");

  return params;
}

INSPressureGradientBC::INSPressureGradientBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters),
    _pressure(adCoupledValue(NS::pressure)),
    _component(getParam<unsigned>("component"))
{
}

ADReal
INSPressureGradientBC::computeQpResidual()
{
  return _pressure[_qp] * _normals[_qp](_component) * _test[_i][_qp];
}
