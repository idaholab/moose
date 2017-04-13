/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "DerivativeFunctionMaterialBase.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<DerivativeFunctionMaterialBase>()
{
  InputParameters params = validParams<FunctionMaterialBase>();
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

DerivativeFunctionMaterialBase::DerivativeFunctionMaterialBase(const InputParameters & parameters)
  : FunctionMaterialBase(parameters),
    _third_derivatives(isParamValid("third_derivatives")
                           ? getParam<bool>("third_derivatives")
                           : (getParam<unsigned int>("derivative_order") == 3))
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
    // first derivatives
    _prop_dF[i] = &declarePropertyDerivative<Real>(_F_name, _arg_names[i]);

    // second derivatives
    for (unsigned int j = i; j < _nargs; ++j)
    {
      _prop_d2F[i][j] = _prop_d2F[j][i] =
          &declarePropertyDerivative<Real>(_F_name, _arg_names[i], _arg_names[j]);

      // third derivatives
      if (_third_derivatives)
      {
        for (unsigned int k = j; k < _nargs; ++k)
        {
          // filling all permutations does not cost us much and simplifies access
          // (no need to check i<=j<=k)
          _prop_d3F[i][j][k] = _prop_d3F[k][i][j] = _prop_d3F[j][k][i] = _prop_d3F[k][j][i] =
              _prop_d3F[j][i][k] = _prop_d3F[i][k][j] = &declarePropertyDerivative<Real>(
                  _F_name, _arg_names[i], _arg_names[j], _arg_names[k]);
        }
      }
    }
  }
}

void
DerivativeFunctionMaterialBase::initialSetup()
{
  // set the _prop_* pointers of all material properties that are not beeing used back to NULL
  bool needs_third_derivatives = false;

  if (!_fe_problem.isMatPropRequested(_F_name))
    _prop_F = NULL;

  for (unsigned int i = 0; i < _nargs; ++i)
  {
    if (!_fe_problem.isMatPropRequested(propertyNameFirst(_F_name, _arg_names[i])))
      _prop_dF[i] = NULL;

    // second derivatives
    for (unsigned int j = i; j < _nargs; ++j)
    {
      if (!_fe_problem.isMatPropRequested(
              propertyNameSecond(_F_name, _arg_names[i], _arg_names[j])))
        _prop_d2F[i][j] = _prop_d2F[j][i] = NULL;

      // third derivatives
      if (_third_derivatives)
      {
        for (unsigned int k = j; k < _nargs; ++k)
        {
          if (!_fe_problem.isMatPropRequested(
                  propertyNameThird(_F_name, _arg_names[i], _arg_names[j], _arg_names[k])))
            _prop_d3F[i][j][k] = _prop_d3F[k][i][j] = _prop_d3F[j][k][i] = _prop_d3F[k][j][i] =
                _prop_d3F[j][i][k] = _prop_d3F[i][k][j] = NULL;
          else
            needs_third_derivatives = true;
        }

        if (!needs_third_derivatives)
          mooseWarning("This simulation does not actually need the third derivatives of "
                       "DerivativeFunctionMaterialBase " +
                       name());
      }
    }
  }
}

void
DerivativeFunctionMaterialBase::computeProperties()
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
