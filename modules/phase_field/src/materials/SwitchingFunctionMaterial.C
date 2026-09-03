//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SwitchingFunctionMaterial.h"

registerMooseObject("PhaseFieldApp", SwitchingFunctionMaterial);
registerMooseObject("PhaseFieldApp", ADSwitchingFunctionMaterial);

template <bool is_ad>
InputParameters
SwitchingFunctionMaterialTempl<is_ad>::validParams()
{
  InputParameters params = OrderParameterFunctionMaterialTempl<is_ad>::validParams();
  params.addClassDescription(
      "Helper material to provide $h(\\eta)$ and its derivative in one of two "
      "polynomial forms.\nSIMPLE: $3\\eta^2-2\\eta^3$\nHIGH: "
      "$\\eta^3(6\\eta^2-15\\eta+10)$");
  MooseEnum h_order("SIMPLE=0 HIGH", "SIMPLE");
  params.addParam<MooseEnum>(
      "h_order", h_order, "Polynomial order of the switching function h(eta)");
  params.set<std::string>("function_name") = std::string("h");
  return params;
}

template <bool is_ad>
SwitchingFunctionMaterialTempl<is_ad>::SwitchingFunctionMaterialTempl(
    const InputParameters & parameters)
  : OrderParameterFunctionMaterialTempl<is_ad>(parameters),
    _h_order(this->template getParam<MooseEnum>("h_order"))
{
}

template <bool is_ad>
void
SwitchingFunctionMaterialTempl<is_ad>::computeQpProperties()
{
  GenericReal<is_ad> n = this->_eta[this->_qp];
  n = n > 1 ? 1 : (n < 0 ? 0 : n);

  switch (_h_order)
  {
    case 0: // SIMPLE
      this->_prop_f[this->_qp] = 3.0 * n * n - 2.0 * n * n * n;
      this->_prop_df[this->_qp] = 6.0 * n - 6.0 * n * n;
      this->_prop_d2f[this->_qp] = 6.0 - 12.0 * n;
      break;

    case 1: // HIGH
      this->_prop_f[this->_qp] = n * n * n * (6.0 * n * n - 15.0 * n + 10.0);
      this->_prop_df[this->_qp] = 30.0 * n * n * (n * n - 2.0 * n + 1.0);
      this->_prop_d2f[this->_qp] = n * (120.0 * n * n - 180.0 * n + 60.0);
      break;

    default:
      this->mooseError("Internal error");
  }
}

template class SwitchingFunctionMaterialTempl<false>;
template class SwitchingFunctionMaterialTempl<true>;
