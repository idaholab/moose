//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

/**
 * This kernel is used for testing derivatives of a material property.
 */
template <typename T>
class MaterialDerivativeTestKernelBase
  : public DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>
{
public:
  static InputParameters validParams();

  MaterialDerivativeTestKernelBase(const InputParameters & parameters);

protected:
  /// number of nonlinear variables
  const unsigned int _n_vars;

  /// select material property derivative to test derivatives of
  std::vector<SymbolName> _derivative;

  /// material property for which to test derivatives
  const MaterialProperty<T> & _p;

  /// material properties for the off-diagonal derivatives of the tested property
  std::vector<const MaterialProperty<T> *> _p_off_diag_derivatives;

  /// material property for the diagonal derivative of the tested property
  const MaterialProperty<T> & _p_diag_derivative;
};

template <typename T>
MaterialDerivativeTestKernelBase<T>::MaterialDerivativeTestKernelBase(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>(parameters),
    _n_vars(_coupled_moose_vars.size()),
    _derivative(getParam<std::vector<SymbolName>>("derivative")),
    _p(this->template getMaterialPropertyDerivative<T>("material_property", _derivative)),
    _p_off_diag_derivatives(_n_vars),
    _p_diag_derivative(this->template getMaterialPropertyDerivative<T>(
        "material_property", MooseUtils::concatenate(_derivative, SymbolName(_var.name()))))
{
  for (unsigned int m = 0; m < _n_vars; ++m)
    _p_off_diag_derivatives[m] = &this->template getMaterialPropertyDerivative<T>(
        "material_property",
        MooseUtils::concatenate(_derivative, SymbolName(_coupled_moose_vars[m]->name())));
}

template <typename T>
InputParameters
MaterialDerivativeTestKernelBase<T>::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Class used for testing derivatives of a material property.");
  params.addRequiredParam<MaterialPropertyName>(
      "material_property", "Name of material property for which derivatives are to be tested.");
  params.addRequiredCoupledVar("args", "List of variables the material property depends on");
  params.deprecateCoupledVar("args", "coupled_variables", "02/07/2024");
  params.addParam<std::vector<SymbolName>>(
      "derivative",
      "Select derivative to test derivatives of (leave empty for checking "
      "derivatives of the original material property)");
  return params;
}
