//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCoupledSwitchingTimeDerivative.h"

registerMooseObject("PhaseFieldApp", ADCoupledSwitchingTimeDerivative);

InputParameters
ADCoupledSwitchingTimeDerivative::validParams()
{
  InputParameters params = ADCoupledTimeDerivative::validParams();
  params.addClassDescription(
      "Coupled time derivative Kernel that multiplies the time derivative by "
      "$\\frac{dh_\\alpha}{d\\eta_i} F_\\alpha + \\frac{dh_\\beta}{d\\eta_i} F_\\beta + \\dots)$");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "Fj_names", "List of functions for each phase. Place in same order as hj_names!");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "hj_names", "Switching Function Materials that provide h. Place in same order as Fj_names!");
  params.addCoupledVar("args", "Vector of arguments of Fj and hj");
  return params;
}

ADCoupledSwitchingTimeDerivative::ADCoupledSwitchingTimeDerivative(
    const InputParameters & parameters)
  : ADCoupledTimeDerivative(parameters),
    _v_name(getVar("v", 0)->name()),
    _Fj_names(getParam<std::vector<MaterialPropertyName>>("Fj_names")),
    _num_j(_Fj_names.size()),
    _prop_Fj(_num_j),
    _hj_names(getParam<std::vector<MaterialPropertyName>>("hj_names")),
    _prop_dhjdetai(_num_j)
{
  // check passed in parameter vectors
  if (_num_j != _hj_names.size())
    paramError("hj_names", "Need to pass in as many hj_names as Fj_names");

  // reserve space and set phase material properties
  for (unsigned int n = 0; n < _num_j; ++n)
  {
    // get phase free energy and derivatives
    _prop_Fj[n] = &getADMaterialProperty<Real>(_Fj_names[n]);

    // get switching function and derivatives wrt eta_i, the nonlinear variable
    _prop_dhjdetai[n] =
        &getADMaterialProperty<Real>(derivativePropertyNameFirst(_hj_names[n], _v_name));
  }
}

// void
// ADCoupledSwitchingTimeDerivative::initialSetup()
// {
//   for (unsigned int n = 0; n < _num_j; ++n)
//     validateNonlinearCoupling<Real>(_hj_names[n]);
// }

ADReal
ADCoupledSwitchingTimeDerivative::precomputeQpResidual()
{
  ADReal sum = 0.0;
  for (unsigned int n = 0; n < _num_j; ++n)
    sum += (*_prop_dhjdetai[n])[_qp] * (*_prop_Fj[n])[_qp];

  return _v_dot[_qp] * sum;
}
