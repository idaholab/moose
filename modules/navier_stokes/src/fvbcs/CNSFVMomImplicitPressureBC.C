//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CNSFVMomImplicitPressureBC.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", CNSFVMomImplicitPressureBC);

InputParameters
CNSFVMomImplicitPressureBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this BC applies to.");
  params.addParam<bool>(
      "include_porosity", false, "Whether to multiply the pressure times porosity");
  params.addClassDescription("Adds an implicit pressure flux contribution on the boundary using "
                             "interior cell information");
  return params;
}

CNSFVMomImplicitPressureBC::CNSFVMomImplicitPressureBC(const InputParameters & parameters)
  : FVFluxBC(parameters),
    _eps(getParam<bool>("include_porosity") ? &getMaterialProperty<Real>(NS::porosity) : nullptr),
    _pressure(getADMaterialProperty<Real>(NS::pressure)),
    _index(getParam<MooseEnum>("momentum_component"))
{
}

ADReal
CNSFVMomImplicitPressureBC::computeQpResidual()
{
  auto resid = _normal(_index) * _pressure[_qp];
  if (_eps)
    resid *= (*_eps)[_qp];
  return resid;
}
