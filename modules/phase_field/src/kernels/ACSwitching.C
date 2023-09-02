//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ACSwitching.h"

registerMooseObject("PhaseFieldApp", ACSwitching);
registerMooseObject("PhaseFieldApp", ADACSwitching);

template <bool is_ad>
InputParameters
ACSwitchingTempl<is_ad>::validParams()
{
  InputParameters params = ACSwitchingBase<is_ad>::validParams();
  params.addClassDescription(
      "Kernel for Allen-Cahn equation that adds derivatives of switching functions and energies");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "Fj_names", "List of free energies for each phase. Place in same order as hj_names!");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "hj_names", "Switching Function Materials that provide h. Place in same order as Fj_names!");
  return params;
}

template <bool is_ad>
ACSwitchingTempl<is_ad>::ACSwitchingTempl(const InputParameters & parameters)
  : ACSwitchingBase<is_ad>(parameters),
    _etai_name(_var.name()),
    _Fj_names(this->template getParam<std::vector<MaterialPropertyName>>("Fj_names")),
    _num_j(_Fj_names.size()),
    _prop_Fj(_num_j),
    _hj_names(this->template getParam<std::vector<MaterialPropertyName>>("hj_names")),
    _prop_dhjdetai(_num_j)
{
  // check passed in parameter vectors
  if (_num_j != _hj_names.size())
    this->paramError("hj_names", "Need to pass in as many hj_names as Fj_names");

  // reserve space and set phase material properties
  for (unsigned int n = 0; n < _num_j; ++n)
  {
    // get phase free energy
    _prop_Fj[n] = &this->template getGenericMaterialProperty<Real, is_ad>(_Fj_names[n]);

    // get first derivative of the switching functions
    _prop_dhjdetai[n] =
        &this->template getMaterialPropertyDerivative<Real, is_ad>(_hj_names[n], _etai_name);
  }
}

ACSwitching::ACSwitching(const InputParameters & parameters)
  : ACSwitchingTempl<false>(parameters),
    _prop_dFjdarg(_num_j),
    _prop_d2hjdetai2(_num_j),
    _prop_d2hjdetaidarg(_num_j)
{
  // reserve space and set phase material properties
  for (unsigned int n = 0; n < _num_j; ++n)
  {
    // get derivatives of the phase free energy and the switching functions
    _prop_dFjdarg[n].resize(_n_args);
    _prop_d2hjdetai2[n] =
        &getMaterialPropertyDerivative<Real>(_hj_names[n], _etai_name, _etai_name);
    _prop_d2hjdetaidarg[n].resize(_n_args);

    for (unsigned int i = 0; i < _n_args; ++i)
    {
      // Get derivatives of all Fj wrt all coupled variables
      _prop_dFjdarg[n][i] = &getMaterialPropertyDerivative<Real>(_Fj_names[n], i);

      // Get second derivatives of all hj wrt eta_i and all coupled variables
      _prop_d2hjdetaidarg[n][i] = &getMaterialPropertyDerivative<Real>(_hj_names[n], _etai_name, i);
    }
  }
}

template <bool is_ad>
void
ACSwitchingTempl<is_ad>::initialSetup()
{
  ACSwitchingBase<is_ad>::initialSetup();
  for (unsigned int n = 0; n < _num_j; ++n)
    this->template validateNonlinearCoupling<GenericReal<is_ad>>(_hj_names[n]);
}

void
ACSwitching::initialSetup()
{
  ACSwitchingTempl<false>::initialSetup();
  for (unsigned int n = 0; n < _num_j; ++n)
    validateNonlinearCoupling<Real>(_Fj_names[n]);
}

Real
ACSwitching::computeDFDOP(PFFunctionType type)
{
  Real sum = 0.0;

  switch (type)
  {
    case Residual:
      for (unsigned int n = 0; n < _num_j; ++n)
        sum += (*_prop_dhjdetai[n])[_qp] * (*_prop_Fj[n])[_qp];

      return sum;

    case Jacobian:
      for (unsigned int n = 0; n < _num_j; ++n)
        sum += (*_prop_d2hjdetai2[n])[_qp] * (*_prop_Fj[n])[_qp];

      return _phi[_j][_qp] * sum;
  }

  mooseError("Invalid type passed in to ACSwitching::computeDFDOP");
}

ADReal
ADACSwitching::computeDFDOP()
{
  ADReal sum = 0.0;
  for (unsigned int n = 0; n < _num_j; ++n)
    sum += (*_prop_dhjdetai[n])[_qp] * (*_prop_Fj[n])[_qp];
  return sum;
}

Real
ACSwitching::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  // first get dependence of mobility _L on other variables using parent class
  // member function
  Real res = ACBulk<Real>::computeQpOffDiagJacobian(jvar);

  // Then add dependence of ACSwitching on other variables
  Real sum = 0.0;
  for (unsigned int n = 0; n < _num_j; ++n)
    sum += (*_prop_d2hjdetaidarg[n][cvar])[_qp] * (*_prop_Fj[n])[_qp] +
           (*_prop_dhjdetai[n])[_qp] * (*_prop_dFjdarg[n][cvar])[_qp];

  res += _L[_qp] * sum * _phi[_j][_qp] * _test[_i][_qp];

  return res;
}

template class ACSwitchingTempl<false>;
template class ACSwitchingTempl<true>;
