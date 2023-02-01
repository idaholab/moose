//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DerivativeFunctionMaterialBase.h"

#include "libmesh/quadrature.h"

template <bool is_ad>
InputParameters
DerivativeFunctionMaterialBaseTempl<is_ad>::validParams()
{

  InputParameters params = FunctionMaterialBase<is_ad>::validParams();
  params.addClassDescription("Material to provide a function (such as a free energy) and its "
                             "derivatives w.r.t. the coupled variables");
  params.addDeprecatedParam<bool>("third_derivatives",
                                  "Flag to indicate if third derivatives are needed",
                                  "Use derivative_order instead.");
  params.addRangeCheckedParam<unsigned int>("derivative_order",
                                            3,
                                            "derivative_order>=2 & derivative_order<=3",
                                            "Maximum order of derivatives taken (2 or 3)");
  return params;
}

template <bool is_ad>
DerivativeFunctionMaterialBaseTempl<is_ad>::DerivativeFunctionMaterialBaseTempl(
    const InputParameters & parameters)
  : FunctionMaterialBase<is_ad>(parameters),
    _third_derivatives(this->template getParam<unsigned int>("derivative_order") == 3)
{
  // reserve space for material properties and explicitly initialize to NULL
  _prop_dF.resize(_nargs, NULL);
  _prop_d2F.resize(_nargs);
  _prop_d3F.resize(_nargs);
  for (unsigned int i = 0; i < _nargs; ++i)
  {
    _prop_d2F[i].resize(_nargs, NULL);

    if (_third_derivatives)
    {
      _prop_d3F[i].resize(_nargs);

      for (unsigned int j = 0; j < _nargs; ++j)
        _prop_d3F[i][j].resize(_nargs, NULL);
    }
  }

  // initialize derivatives
  for (unsigned int i = 0; i < _nargs; ++i)
  {
    if (!Coupleable::isCoupledConstant(_arg_names[i]))
      _prop_dF[i] = &this->template declarePropertyDerivative<Real, is_ad>(_F_name, _arg_names[i]);

    // second derivatives
    for (unsigned int j = i; j < _nargs; ++j)
    {
      if (!Coupleable::isCoupledConstant(_arg_names[i]))
        _prop_d2F[i][j] = _prop_d2F[j][i] = &this->template declarePropertyDerivative<Real, is_ad>(
            _F_name, _arg_names[i], _arg_names[j]);

      // third derivatives
      if (_third_derivatives)
      {
        for (unsigned int k = j; k < _nargs; ++k)
        {
          // filling all permutations does not cost us much and simplifies access
          // (no need to check i<=j<=k)
          if (!Coupleable::isCoupledConstant(_arg_names[i]))
            _prop_d3F[i][j][k] = _prop_d3F[k][i][j] = _prop_d3F[j][k][i] = _prop_d3F[k][j][i] =
                _prop_d3F[j][i][k] = _prop_d3F[i][k][j] =
                    &this->template declarePropertyDerivative<Real, is_ad>(
                        _F_name, _arg_names[i], _arg_names[j], _arg_names[k]);
        }
      }
    }
  }
}

template <bool is_ad>
void
DerivativeFunctionMaterialBaseTempl<is_ad>::initialSetup()
{
  // set the _prop_* pointers of all material properties that are not beeing used back to NULL
  bool needs_third_derivatives = false;

  if (!_fe_problem.isMatPropRequested(_F_name))
    _prop_F = NULL;

  for (unsigned int i = 0; i < _nargs; ++i)
  {
    if (!_fe_problem.isMatPropRequested(this->derivativePropertyNameFirst(_F_name, _arg_names[i])))
      _prop_dF[i] = NULL;

    // second derivatives
    for (unsigned int j = i; j < _nargs; ++j)
    {
      if (!_fe_problem.isMatPropRequested(
              this->derivativePropertyNameSecond(_F_name, _arg_names[i], _arg_names[j])))
        _prop_d2F[i][j] = _prop_d2F[j][i] = NULL;

      // third derivatives
      if (_third_derivatives)
      {
        for (unsigned int k = j; k < _nargs; ++k)
        {
          if (!_fe_problem.isMatPropRequested(this->derivativePropertyNameThird(
                  _F_name, _arg_names[i], _arg_names[j], _arg_names[k])))
            _prop_d3F[i][j][k] = _prop_d3F[k][i][j] = _prop_d3F[j][k][i] = _prop_d3F[k][j][i] =
                _prop_d3F[j][i][k] = _prop_d3F[i][k][j] = NULL;
          else
            needs_third_derivatives = true;
        }

        if (!needs_third_derivatives)
          mooseWarning("This simulation does not actually need the third derivatives of "
                       "DerivativeFunctionMaterialBaseTempl " +
                       name());
      }
    }
  }
}

template <bool is_ad>
void
DerivativeFunctionMaterialBaseTempl<is_ad>::computeProperties()
{
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    // set function value
    if (_prop_F)
      (*_prop_F)[_qp] = computeF();

    for (unsigned int i = 0; i < _nargs; ++i)
    {
      // set first derivatives
      if (_prop_dF[i])
        (*_prop_dF[i])[_qp] = computeDF(_arg_numbers[i]);

      // second derivatives
      for (unsigned int j = i; j < _nargs; ++j)
      {
        if (_prop_d2F[i][j])
          (*_prop_d2F[i][j])[_qp] = computeD2F(_arg_numbers[i], _arg_numbers[j]);

        // third derivatives
        if (_third_derivatives)
        {
          for (unsigned int k = j; k < _nargs; ++k)
            if (_prop_d3F[i][j][k])
              (*_prop_d3F[i][j][k])[_qp] =
                  computeD3F(_arg_numbers[i], _arg_numbers[j], _arg_numbers[k]);
        }
      }
    }
  }
}

// explicit instantiation
template class DerivativeFunctionMaterialBaseTempl<true>;
template class DerivativeFunctionMaterialBaseTempl<false>;
