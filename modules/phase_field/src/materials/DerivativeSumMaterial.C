#include "DerivativeSumMaterial.h"

template<>
InputParameters validParams<Material>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Meta-material to sum up multiple derivative materials");
  params.addParam<std::vector<std::string> >("f_names", "F", "Base name of the free energy function (used to name the material properties)");
  //params.addParam<bool>("third_derivatives", true, "Calculate third derivatoves of the free energy");

  // All arguments to the free energies being summed
  params.addRequiredCoupledVar("args", "Arguments of the free energy functions being summed - use vector coupling");

  return params;
}

DerivativeSumMaterial::DerivativeSumMaterial(const std::string & name,
                                             InputParameters parameters) :
    DerivativeBaseMaterial(name, parameters),
    _f_names(getParam<std::vector<std::string> >("f_names")),
    _num_materials(_f_names.size())
{
  // we need at least one material in the sum
  if (_num_materials == 0)
    mooseError("Please supply at least one material to sum in DerivativeSumMaterial " << name);

  // reserve space for summand material properties
  _prop_F.resize(_num_materials);
  _prop_dF.resize(_num_materials);
  _prop_d2F.resize(_num_materials);
  _prop_d3F.resize(_num_materials);

  for (unsigned int n = 0; n < _num_materials; ++n)
  {
    _prop_F[n] = & getMaterialProperty<Real>(_f_names[n]);
    _prop_dF[n].resize(_nargs);
    _prop_d2F[n].resize(_nargs);
    _prop_d3F[n].resize(_nargs);

    for (unsigned int i = 0; i < _nargs; ++i)
    {
      _prop_dF[n][i] = &getMaterialPropertyDerivative<Real>(_f_names[n], _arg_names[i]);
      _prop_d2F[n][i].resize(_nargs);

      if (_third_derivatives)
        _prop_d3F[n][i].resize(_nargs);

      for (unsigned int j = 0; j < _nargs; ++j)
      {
        _prop_d2F[n][i][j] = &getMaterialPropertyDerivative<Real>(_f_names[n], _arg_names[i], _arg_names[j]);

        if (_third_derivatives) {
          _prop_d3F[n][i][j].resize(_nargs);

          for (unsigned int k = 0; k < _nargs; ++k)
            _prop_d3F[n][i][j][k] = &getMaterialPropertyDerivative<Real>(_f_names[n], _arg_names[i], _arg_names[j], _arg_names[k]);
        }
      }
    }
  }
}


// TODO: we'd better overwrite computeProperties!
Real
DerivativeSumMaterial::computeF()
{
  Real sum = (*_prop_F[0])[_qp];
  for (unsigned int n = 1; n < _num_materials; ++n)
    sum += (*_prop_F[n])[_qp];
  return sum;
}

Real
DerivativeSumMaterial::computeDF(unsigned int i_var)
{
  unsigned int i = argIndex(i_var);

  Real sum = (*_prop_dF[0][i])[_qp];
  for (unsigned int n = 1; n < _num_materials; ++n)
    sum += (*_prop_dF[n][i])[_qp];
  return sum;
}

Real
DerivativeSumMaterial::computeD2F(unsigned int i_var, unsigned int j_var)
{
  unsigned int i = argIndex(i_var);
  unsigned int j = argIndex(j_var);

  Real sum = (*_prop_d2F[0][i][j])[_qp];
  for (unsigned int n = 1; n < _num_materials; ++n)
    sum += (*_prop_d2F[n][i][j])[_qp];
  return sum;
}

Real
DerivativeSumMaterial::computeD3F(unsigned int i_var, unsigned int j_var, unsigned int k_var)
{
  unsigned int i = argIndex(i_var);
  unsigned int j = argIndex(j_var);
  unsigned int k = argIndex(k_var);

  Real sum = (*_prop_d3F[0][i][j][k])[_qp];
  for (unsigned int n = 1; n < _num_materials; ++n)
    sum += (*_prop_d3F[n][i][j][k])[_qp];
  return sum;
}
