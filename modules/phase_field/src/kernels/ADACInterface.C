//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADACInterface.h"

registerMooseObject("PhaseFieldApp", ADACInterface);

InputParameters
ADACInterface::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription("Gradient energy Allen-Cahn Kernel");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  params.addParam<MaterialPropertyName>("kappa_name", "kappa_op", "The kappa used with the kernel");
  params.addCoupledVar("args", "Vector of nonlinear variable arguments this object depends on");
  params.deprecateCoupledVar("args", "coupled_variables", "02/27/2024");

  params.addParam<bool>("variable_L",
                        true,
                        "The mobility is a function of any MOOSE variable (if "
                        "this is set to false L must be constant over the "
                        "entire domain!)");
  return params;
}

ADACInterface::ADACInterface(const InputParameters & parameters)
  : ADKernel(parameters),
    _prop_L(getADMaterialProperty<Real>("mob_name")),
    _name_L(getParam<MaterialPropertyName>("mob_name")),
    _kappa(getADMaterialProperty<Real>("kappa_name")),
    _variable_L(getParam<bool>("variable_L")),
    _dLdop(_variable_L
               ? &getADMaterialProperty<Real>(derivativePropertyNameFirst(_name_L, _var.name()))
               : nullptr),
    _nvar(Coupleable::_coupled_standard_moose_vars.size()),
    _dLdarg(_nvar),
    _gradarg(_nvar)
{
  // Get mobility and kappa derivatives and coupled variable gradients
  if (_variable_L)
    for (unsigned int i = 0; i < _nvar; ++i)
    {
      MooseVariable * ivar = _coupled_standard_moose_vars[i];
      const VariableName iname = ivar->name();
      if (iname == _var.name())
      {
        if (isCoupled("args"))
          paramError(
              "args",
              "The kernel variable should not be specified in the coupled `args` parameter.");
        else
          paramError(
              "coupled_variables",
              "The kernel variable should not be specified in the coupled `coupled_variables` "
              "parameter.");
      }

      _dLdarg[i] = &getADMaterialProperty<Real>(derivativePropertyNameFirst(_name_L, iname));
      _gradarg[i] = &(ivar->adGradSln());
    }
}

ADReal
ADACInterface::computeQpResidual()
{
  // nabla_Lpsi is the product rule gradient \f$ \nabla (L\psi) \f$
  ADRealVectorValue nabla_Lpsi = _prop_L[_qp] * _grad_test[_i][_qp];

  if (_variable_L)
  {
    ADRealVectorValue grad_L = _grad_u[_qp] * (*_dLdop)[_qp];
    for (unsigned int i = 0; i < _nvar; ++i)
      grad_L += (*_gradarg[i])[_qp] * (*_dLdarg[i])[_qp];

    nabla_Lpsi += grad_L * _test[_i][_qp];
  }

  return _grad_u[_qp] * _kappa[_qp] * nabla_Lpsi;
}
