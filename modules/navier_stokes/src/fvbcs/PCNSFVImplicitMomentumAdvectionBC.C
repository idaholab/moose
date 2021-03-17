//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PCNSFVImplicitMomentumAdvectionBC.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PCNSFVImplicitMomentumAdvectionBC);

InputParameters
PCNSFVImplicitMomentumAdvectionBC::validParams()
{
  InputParameters params = PCNSFVImplicitMassBC::validParams();
  params.addClassDescription("Implicit momentum advection BC.");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  return params;
}

PCNSFVImplicitMomentumAdvectionBC::PCNSFVImplicitMomentumAdvectionBC(const InputParameters & params)
  : PCNSFVImplicitMassBC(params),
    _velocity(getADMaterialProperty<RealVectorValue>(NS::velocity)),
    _index(getParam<MooseEnum>("momentum_component"))
{
}

ADReal
PCNSFVImplicitMomentumAdvectionBC::computeQpResidual()
{
  return PCNSFVImplicitMomentumAdvectionBC::computeQpResidual() * _velocity[_qp](_index);
}
