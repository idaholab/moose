//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConstantDensityThermalSolidPropertiesMaterial.h"
#include "ThermalSolidProperties.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("SolidPropertiesApp", ConstantDensityThermalSolidPropertiesMaterial);
registerMooseObject("SolidPropertiesApp", ADConstantDensityThermalSolidPropertiesMaterial);

template <bool is_ad>
InputParameters
ConstantDensityThermalSolidPropertiesMaterialTempl<is_ad>::validParams()
{
  InputParameters params = ThermalSolidPropertiesMaterialTempl<is_ad>::validParams();

  params.addRequiredParam<Real>("T_ref", "Reference temperature for constant density [K]");

  params.addClassDescription("Computes solid thermal properties as a function of temperature but "
                             "with a constant density.");

  return params;
}

template <bool is_ad>
ConstantDensityThermalSolidPropertiesMaterialTempl<
    is_ad>::ConstantDensityThermalSolidPropertiesMaterialTempl(const InputParameters & parameters)
  : ThermalSolidPropertiesMaterialTempl<is_ad>(parameters),

    _T_ref(this->template getParam<Real>("T_ref")),
    _rho_constant(MetaPhysicL::raw_value(_sp.rho_from_T(GenericReal<is_ad>(_T_ref))))
{
}

template <bool is_ad>
void
ConstantDensityThermalSolidPropertiesMaterialTempl<is_ad>::computeQpProperties()
{
  _cp[_qp] = _sp.cp_from_T(_temperature[_qp]);
  _k[_qp] = _sp.k_from_T(_temperature[_qp]);
  _rho[_qp] = _rho_constant;
}

template class ConstantDensityThermalSolidPropertiesMaterialTempl<false>;
template class ConstantDensityThermalSolidPropertiesMaterialTempl<true>;
