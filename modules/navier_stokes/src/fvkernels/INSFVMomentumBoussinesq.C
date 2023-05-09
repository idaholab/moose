//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMomentumBoussinesq.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSFVMomentumBoussinesq);

InputParameters
INSFVMomentumBoussinesq::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params += INSFVMomentumResidualObject::validParams();
  params.addClassDescription("Computes a body force for natural convection buoyancy.");
  params.addRequiredParam<MooseFunctorName>(NS::T_fluid, "the fluid temperature");
  params.addRequiredParam<RealVectorValue>("gravity", "Direction of the gravity vector");
  params.addParam<MooseFunctorName>("alpha_name",
                                    NS::alpha_boussinesq,
                                    "The name of the thermal expansion coefficient"
                                    "this is of the form rho = rho*(1-alpha (T-T_ref))");
  params.addRequiredParam<Real>("ref_temperature", "The value for the reference temperature.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "The value for the density");
  return params;
}

INSFVMomentumBoussinesq::INSFVMomentumBoussinesq(const InputParameters & params)
  : FVElementalKernel(params),
    INSFVMomentumResidualObject(*this),
    _temperature(getFunctor<ADReal>(NS::T_fluid)),
    _gravity(getParam<RealVectorValue>("gravity")),
    _alpha(getFunctor<ADReal>("alpha_name")),
    _ref_temperature(getParam<Real>("ref_temperature")),
    _rho(getFunctor<ADReal>(NS::density))
{
  if (!_rho.isConstant())
    paramError(NS::density, "The density in the boussinesq term is not constant!");
}

ADReal
INSFVMomentumBoussinesq::computeQpResidual()
{
  const auto elem = makeElemArg(_current_elem);
  const auto state = determineState();
  return _alpha(elem, state) * _gravity(_index) * _rho(elem, state) *
         (_temperature(elem, state) - _ref_temperature);
}
