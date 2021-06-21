//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PCNSFVImplicitMomentumPressureBC.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PCNSFVImplicitMomentumPressureBC);

InputParameters
PCNSFVImplicitMomentumPressureBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addClassDescription(
      "Specifies an implicit pressure at a boundary for the momentum equations.");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  return params;
}

PCNSFVImplicitMomentumPressureBC::PCNSFVImplicitMomentumPressureBC(const InputParameters & params)
  : FVFluxBC(params),
    _pressure(getADMaterialProperty<Real>(NS::pressure)),
    _eps(getMaterialProperty<Real>(NS::porosity)),
    _index(getParam<MooseEnum>("momentum_component"))
{
}

ADReal
PCNSFVImplicitMomentumPressureBC::computeQpResidual()
{

  return _normal(_index) * _eps[_qp] * _pressure[_qp];
}
