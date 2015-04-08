/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "DerivativeSumMaterial.h"

template<>
InputParameters validParams<DerivativeSumMaterial>()
{
  InputParameters params = validParams<DerivativeFunctionMaterialBase>();
  params.addClassDescription("Meta-material to sum up multiple derivative materials");
  params.addParam<std::vector<std::string> >("sum_materials", "Base name of the free energy function (used to name the material properties)");
  //params.addParam<bool>("third_derivatives", true, "Calculate third derivatoves of the free energy");

  // All arguments to the free energies being summed
  params.addRequiredCoupledVar("args", "Arguments of the free energy functions being summed - use vector coupling");

  // Advanced arguments to construct a sum of the form \f$ c+\gamma\sum_iF_i \f$
  params.addParam<Real>("prefactor", 1.0, "Prefactor to multiply the sum term with.");
  params.addParam<Real>("constant", 0.0, "Constant to be added to the prefactor multiplied sum.");
  params.addParamNamesToGroup("prefactor constant", "Advanced");

  return params;
}

DerivativeSumMaterial::DerivativeSumMaterial(const std::string & name,
                                             InputParameters parameters) :
    DerivativeFunctionMaterialBase(name, parameters),
    _sum_materials(getParam<std::vector<std::string> >("sum_materials")),
    _num_materials(_sum_materials.size()),
    _prefactor(getParam<Real>("prefactor")),
    _constant(getParam<Real>("constant"))
  {
  // we need at least one material in the sum
  if (_num_materials == 0)
    mooseError("Please supply at least one material to sum in DerivativeSumMaterial " << name);

  // reserve space for summand material properties
  _summand_F.resize(_num_materials);
  _summand_dF.resize(_num_materials);
  _summand_d2F.resize(_num_materials);
  _summand_d3F.resize(_num_materials);

  for (unsigned int n = 0; n < _num_materials; ++n)
  {
    _summand_F[n] = & getMaterialProperty<Real>(_sum_materials[n]);
    _summand_dF[n].resize(_nargs);
    _summand_d2F[n].resize(_nargs);
    _summand_d3F[n].resize(_nargs);

    for (unsigned int i = 0; i < _nargs; ++i)
    {
      _summand_dF[n][i] = &getMaterialPropertyDerivative<Real>(_sum_materials[n], _arg_names[i]);
      _summand_d2F[n][i].resize(_nargs);

      if (_third_derivatives)
        _summand_d3F[n][i].resize(_nargs);

      for (unsigned int j = 0; j < _nargs; ++j)
      {
        _summand_d2F[n][i][j] = &getMaterialPropertyDerivative<Real>(_sum_materials[n], _arg_names[i], _arg_names[j]);

        if (_third_derivatives) {
          _summand_d3F[n][i][j].resize(_nargs);

          for (unsigned int k = 0; k < _nargs; ++k)
            _summand_d3F[n][i][j][k] = &getMaterialPropertyDerivative<Real>(_sum_materials[n], _arg_names[i], _arg_names[j], _arg_names[k]);
        }
      }
    }
  }
}

void
DerivativeSumMaterial::computeProperties()
{
  unsigned int i, j, k;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    // set function value
    if (_prop_F) {
      (*_prop_F)[_qp] = (*_summand_F[0])[_qp];
      for (unsigned int n = 1; n < _num_materials; ++n)
        (*_prop_F)[_qp] += (*_summand_F[n])[_qp];
    }

    for (i = 0; i < _nargs; ++i)
    {
      // set first derivatives
      if (_prop_dF[i]) {
        (*_prop_dF[i])[_qp] =  (*_summand_dF[0][i])[_qp];
        for (unsigned int n = 1; n < _num_materials; ++n)
          (*_prop_dF[i])[_qp] += (*_summand_dF[n][i])[_qp];
      }

      // second derivatives
      for (j = i; j < _nargs; ++j)
      {
        if (_prop_d2F[i][j]) {
          (*_prop_d2F[i][j])[_qp] =  (*_summand_d2F[0][i][j])[_qp];
          for (unsigned int n = 1; n < _num_materials; ++n)
            (*_prop_d2F[i][j])[_qp] += (*_summand_d2F[n][i][j])[_qp];
          }

        // third derivatives
        if (_third_derivatives)
        {
          for (k = j; k < _nargs; ++k)
            if (_prop_d3F[i][j][k]) {
              (*_prop_d3F[i][j][k])[_qp] =  (*_summand_d3F[0][i][j][k])[_qp];
              for (unsigned int n = 1; n < _num_materials; ++n)
                (*_prop_d3F[i][j][k])[_qp] += (*_summand_d3F[n][i][j][k])[_qp];
            }
        }
      }
    }
  }
}
