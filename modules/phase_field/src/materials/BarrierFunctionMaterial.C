//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BarrierFunctionMaterial.h"

registerMooseObject("PhaseFieldApp", BarrierFunctionMaterial);
registerMooseObject("PhaseFieldApp", ADBarrierFunctionMaterial);

template <bool is_ad>
InputParameters
BarrierFunctionMaterialTempl<is_ad>::validParams()
{
  InputParameters params = OrderParameterFunctionMaterialTempl<is_ad>::validParams();
  params.addClassDescription("Helper material to provide $g(\\eta)$ and its derivative in a "
                             "polynomial.\nSIMPLE: $\\eta^2(1-\\eta)^2$\nLOW: $\\eta(1-\\eta)$"
                             "\nHIGH: $\\eta^2(1-\\eta^2)^2$");
  MooseEnum g_order("SIMPLE=0 LOW HIGH", "SIMPLE");
  params.addParam<MooseEnum>("g_order", g_order, "Polynomial order of the barrier function g(eta)");
  params.addParam<bool>("well_only",
                        false,
                        "Make the g zero in [0:1] so it only contributes to "
                        "enforcing the eta range and not to the phase "
                        "transformation barrier.");
  params.set<std::string>("function_name") = std::string("g");
  return params;
}

template <bool is_ad>
BarrierFunctionMaterialTempl<is_ad>::BarrierFunctionMaterialTempl(
    const InputParameters & parameters)
  : OrderParameterFunctionMaterialTempl<is_ad>(parameters),
    _g_order(this->template getParam<MooseEnum>("g_order")),
    _well_only(this->template getParam<bool>("well_only"))
{
}

template <bool is_ad>
void
BarrierFunctionMaterialTempl<is_ad>::computeQpProperties()
{
  const auto & n = this->_eta[this->_qp];

  if (_well_only && MetaPhysicL::raw_value(n) >= 0.0 && MetaPhysicL::raw_value(n) <= 1.0)
  {
    this->_prop_f[this->_qp] = 0.0;
    this->_prop_df[this->_qp] = 0.0;
    this->_prop_d2f[this->_qp] = 0.0;
    return;
  }

  switch (_g_order)
  {
    case 0: // SIMPLE
      this->_prop_f[this->_qp] = n * n * (1.0 - n) * (1.0 - n);
      this->_prop_df[this->_qp] = 2.0 * n * (n - 1.0) * (2.0 * n - 1.0);
      this->_prop_d2f[this->_qp] = 12.0 * (n * n - n) + 2.0;
      break;

    case 1: // LOW
      this->_prop_f[this->_qp] = n * (1.0 - n);
      this->_prop_df[this->_qp] = 1.0 - 2.0 * n;
      this->_prop_d2f[this->_qp] = -2.0;
      break;

    case 2: // HIGH
      this->_prop_f[this->_qp] = n * n * (1.0 - n * n) * (1.0 - n * n);
      this->_prop_df[this->_qp] = 2 * n * (1 - n * n * (4 - 3 * n * n));
      this->_prop_d2f[this->_qp] = 2 - 6 * n * n * (4 - 5 * n * n);
      break;

    default:
      this->mooseError("Internal error");
  }
}

template class BarrierFunctionMaterialTempl<false>;
template class BarrierFunctionMaterialTempl<true>;