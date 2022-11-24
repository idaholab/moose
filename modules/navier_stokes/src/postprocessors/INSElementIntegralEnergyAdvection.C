//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSElementIntegralEnergyAdvection.h"
#include "NS.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("NavierStokesApp", INSElementIntegralEnergyAdvection);
registerMooseObject("NavierStokesApp", INSADElementIntegralEnergyAdvection);

template <bool is_ad>
InputParameters
INSElementIntegralEnergyAdvectionTempl<is_ad>::validParams()
{
  InputParameters params = ElementIntegralPostprocessor::validParams();
  params.addClassDescription(
      "Computes the net volumetric balance of energy transported by advection");
  params.addRequiredParam<MaterialPropertyName>(NS::cp,
                                                "The constant-pressure specific heat capacity");
  params.addRequiredParam<MaterialPropertyName>(NS::density, "The density");
  params.addRequiredCoupledVar(NS::temperature, "The temperature");
  params.addRequiredCoupledVar(NS::velocity, "The velocity");
  return params;
}

template <bool is_ad>
INSElementIntegralEnergyAdvectionTempl<is_ad>::INSElementIntegralEnergyAdvectionTempl(
    const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    _cp(getGenericMaterialProperty<Real, is_ad>(NS::cp)),
    _rho(getGenericMaterialProperty<Real, is_ad>(NS::density)),
    _grad_T(coupledGradient(NS::temperature)),
    _velocity(coupledVectorValue(NS::velocity))
{
}

template <bool is_ad>
Real
INSElementIntegralEnergyAdvectionTempl<is_ad>::computeQpIntegral()
{
  return MetaPhysicL::raw_value(_cp[_qp]) * MetaPhysicL::raw_value(_rho[_qp]) *
         (_grad_T[_qp] * _velocity[_qp]);
}

template class INSElementIntegralEnergyAdvectionTempl<false>;
template class INSElementIntegralEnergyAdvectionTempl<true>;
