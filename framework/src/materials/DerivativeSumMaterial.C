//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DerivativeSumMaterial.h"

#include "libmesh/quadrature.h"

registerMooseObject("MooseApp", DerivativeSumMaterial);
registerMooseObject("MooseApp", ADDerivativeSumMaterial);

template <bool is_ad>
InputParameters
DerivativeSumMaterialTempl<is_ad>::validParams()
{
  InputParameters params = DerivativeFunctionMaterialBaseTempl<is_ad>::validParams();
  params.addClassDescription("Meta-material to sum up multiple derivative materials");
  params.addParam<std::vector<std::string>>("sum_materials",
                                            "Base name of the parsed sum material property");

  // All arguments of the parsed expression (free energy) being summed
  params.addRequiredCoupledVar("args", "Vector of names of variables being summed");
  params.deprecateCoupledVar("args", "coupled_variables", "02/07/2024");

  params.addCoupledVar("displacement_gradients",
                       "Vector of displacement gradient variables (see "
                       "Modules/PhaseField/DisplacementGradients "
                       "action)");

  // Advanced arguments to construct a sum of the form \f$ c+\gamma\sum_iF_i \f$
  params.addParam<std::vector<Real>>("prefactor", "Prefactor to multiply the sum term with.");
  params.addParam<Real>("constant", 0.0, "Constant to be added to the prefactor multiplied sum.");

  params.addParam<bool>("validate_coupling",
                        true,
                        "Check if all variables the specified materials depend on are listed in "
                        "the `coupled_variables` parameter.");
  params.addParamNamesToGroup("prefactor constant", "Advanced");

  return params;
}

template <bool is_ad>
DerivativeSumMaterialTempl<is_ad>::DerivativeSumMaterialTempl(const InputParameters & parameters)
  : DerivativeFunctionMaterialBaseTempl<is_ad>(parameters),
    _sum_materials(this->template getParam<std::vector<std::string>>("sum_materials")),
    _num_materials(_sum_materials.size()),
    _prefactor(_num_materials, 1.0),
    _constant(this->template getParam<Real>("constant")),
    _validate_coupling(this->template getParam<bool>("validate_coupling"))
{
  // we need at least one material in the sum
  if (_num_materials == 0)
    mooseError("Please supply at least one material to sum in DerivativeSumMaterial ", name());

  // get prefactor values if not 1.0
  std::vector<Real> p = this->template getParam<std::vector<Real>>("prefactor");

  // if prefactor is used we need the same number of prefactors as sum materials
  if (_num_materials == p.size())
    _prefactor = p;
  else if (p.size() != 0)
    mooseError("Supply the same number of sum materials and prefactors.");

  // reserve space for summand material properties
  _summand_F.resize(_num_materials);
  _summand_dF.resize(_num_materials);
  _summand_d2F.resize(_num_materials);
  _summand_d3F.resize(_num_materials);

  for (unsigned int n = 0; n < _num_materials; ++n)
  {
    _summand_F[n] = &this->template getGenericMaterialProperty<Real, is_ad>(_sum_materials[n]);
    _summand_dF[n].resize(_nargs);
    _summand_d2F[n].resize(_nargs);
    _summand_d3F[n].resize(_nargs);

    for (unsigned int i = 0; i < _nargs; ++i)
    {
      _summand_dF[n][i] = &this->template getMaterialPropertyDerivative<Real, is_ad>(
          _sum_materials[n], _arg_names[i]);
      _summand_d2F[n][i].resize(_nargs);

      if (_third_derivatives)
        _summand_d3F[n][i].resize(_nargs);

      for (unsigned int j = 0; j < _nargs; ++j)
      {
        _summand_d2F[n][i][j] = &this->template getMaterialPropertyDerivative<Real, is_ad>(
            _sum_materials[n], _arg_names[i], _arg_names[j]);

        if (_third_derivatives)
        {
          _summand_d3F[n][i][j].resize(_nargs);

          for (unsigned int k = 0; k < _nargs; ++k)
            _summand_d3F[n][i][j][k] = &this->template getMaterialPropertyDerivative<Real, is_ad>(
                _sum_materials[n], _arg_names[i], _arg_names[j], _arg_names[k]);
        }
      }
    }
  }
}

template <bool is_ad>
void
DerivativeSumMaterialTempl<is_ad>::initialSetup()
{
  if (_validate_coupling)
    for (unsigned int n = 0; n < _num_materials; ++n)
      this->template validateCoupling<Real>(_sum_materials[n]);
}

template <bool is_ad>
void
DerivativeSumMaterialTempl<is_ad>::computeProperties()
{
  unsigned int i, j, k;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    // set function value
    if (_prop_F)
    {
      (*_prop_F)[_qp] = (*_summand_F[0])[_qp] * _prefactor[0];
      for (unsigned int n = 1; n < _num_materials; ++n)
        (*_prop_F)[_qp] += (*_summand_F[n])[_qp] * _prefactor[n];
    }

    for (i = 0; i < _nargs; ++i)
    {
      // set first derivatives
      if (_prop_dF[i])
      {
        (*_prop_dF[i])[_qp] = (*_summand_dF[0][i])[_qp] * _prefactor[0];
        for (unsigned int n = 1; n < _num_materials; ++n)
          (*_prop_dF[i])[_qp] += (*_summand_dF[n][i])[_qp] * _prefactor[n];
      }

      // second derivatives
      for (j = i; j < _nargs; ++j)
      {
        if (_prop_d2F[i][j])
        {
          (*_prop_d2F[i][j])[_qp] = (*_summand_d2F[0][i][j])[_qp] * _prefactor[0];
          for (unsigned int n = 1; n < _num_materials; ++n)
            (*_prop_d2F[i][j])[_qp] += (*_summand_d2F[n][i][j])[_qp] * _prefactor[n];
        }

        // third derivatives
        if (_third_derivatives)
        {
          for (k = j; k < _nargs; ++k)
            if (_prop_d3F[i][j][k])
            {
              (*_prop_d3F[i][j][k])[_qp] = (*_summand_d3F[0][i][j][k])[_qp] * _prefactor[0];
              for (unsigned int n = 1; n < _num_materials; ++n)
                (*_prop_d3F[i][j][k])[_qp] += (*_summand_d3F[n][i][j][k])[_qp] * _prefactor[n];
            }
        }
      }
    }
  }
}
