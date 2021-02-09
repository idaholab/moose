//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "FVMomImplicitPressureBC.h"
#include "NS.h"

namespace nms = NS;

registerADMooseObject("NavierStokesApp", CNSFVMomImplicitPressureBC);

InputParameters
CNSFVMomImplicitPressureBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addParam<Real>(nms::porosity, 1, "porosity");
  MooseEnum momentum_component("x=0 y=1 z=2", "x");
  params.addParam<MooseEnum>("momentum_component",
                             momentum_component,
                             "The component of the momentum equation that this BC applies to.");
  return params;
}

CNSFVMomImplicitPressureBC::CNSFVMomImplicitPressureBC(const InputParameters & parameters)
  : FVFluxBC(parameters),
    _eps(getParam<Real>(nms::porosity)),
    _pressure(getADMaterialProperty<Real>(nms::pressure)),
    _index(getParam<MooseEnum>("momentum_component"))
{
}

ADReal
CNSFVMomImplicitPressureBC::computeQpResidual()
{
  return _normal(_index) * _pressure[_qp] * _eps;
}
