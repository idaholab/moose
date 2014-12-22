#include "DerivativeBaseMaterial.h"

template<>
InputParameters validParams<DerivativeBaseMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("KKS model helper material to provide the free energy and its first and second derivatives");
  params.addParam<std::string>("f_name", "F", "Base name of the free energy function (used to name the material properties)");
  params.addRequiredCoupledVar("args", "Arguments of F() - use vector coupling");
  params.addParam<bool>("third_derivatives", true, "Calculate third derivatoves of the free energy");
  return params;
}

DerivativeBaseMaterial::DerivativeBaseMaterial(const std::string & name,
                                               InputParameters parameters) :
    DerivativeMaterialInterface<Material>(name, parameters),
    _F_name(getParam<std::string>("f_name")),
    _nargs(_coupled_moose_vars.size()),
    _third_derivatives(getParam<bool>("third_derivatives")),
    _prop_F(&declareProperty<Real>(_F_name))
{
  // loop counters
  unsigned int i, j, k;

  // coupled variables
  _args.resize(_nargs);

  // reserve space for material properties
  _prop_dF.resize(_nargs);
  _prop_d2F.resize(_nargs);
  _prop_d3F.resize(_nargs);
  for (i = 0; i < _nargs; ++i)
  {
    _prop_d2F[i].resize(_nargs);

    // TODO: maybe we should reserve and initialize to NULL...
    if (_third_derivatives) {
      _prop_d3F[i].resize(_nargs);

      for (j = 0; j < _nargs; ++j)
        _prop_d3F[i][j].resize(_nargs);
    }
  }

  // fetch names of variables in args
  _arg_names.resize(_nargs);
  for (i = 0; i < _nargs; ++i)
    _arg_names[i] = _coupled_moose_vars[i]->name();

  // initialize derivatives
  for (i = 0; i < _nargs; ++i)
  {
    // get the coupled variable to use as function arguments
    _args[i] = &coupledValue(_coupled_moose_vars[i]);

    // first derivatives
    _prop_dF[i] = &declarePropertyDerivative<Real>(_F_name, _arg_names[i]);

    // second derivatives
    for (j = i; j < _nargs; ++j)
    {
      _prop_d2F[i][j] =
      _prop_d2F[j][i] = &declarePropertyDerivative<Real>(_F_name, _arg_names[i], _arg_names[j]);

      // third derivatives
      if (_third_derivatives)
      {
        for (k = j; k < _nargs; ++k)
        {
          // filling all permutations does not cost us much and simplifies access
          // (no need to check i<=j<=k)
          _prop_d3F[i][j][k] =
          _prop_d3F[k][i][j] =
          _prop_d3F[j][k][i] =
          _prop_d3F[k][j][i] =
          _prop_d3F[j][i][k] =
          _prop_d3F[i][k][j] = &declarePropertyDerivative<Real>(_F_name, _arg_names[i], _arg_names[j], _arg_names[k]);
        }
      }
    }
  }
}

void
DerivativeBaseMaterial::initialSetup()
{
  if (_nargs != expectedNumArgs()) {
    mooseError("Wrong number of arguments supplied for DerivativeBaseMaterial " + _F_name);
  }

  // set the _prop_* pointers of all material poroperties that are not beeing used back to NULL
  unsigned int i, j, k;
  bool needs_third_derivatives = false;

  if (!_fe_problem.isMatPropRequested(_F_name))
    _prop_F = NULL;

  for (i = 0; i < _nargs; ++i)
  {
    if (!_fe_problem.isMatPropRequested(propertyNameFirst(_F_name, _arg_names[i])))
      _prop_dF[i] = NULL;

    // second derivatives
    for (j = i; j < _nargs; ++j)
    {
      if (!_fe_problem.isMatPropRequested(propertyNameSecond(_F_name, _arg_names[i], _arg_names[j])))
        _prop_d2F[i][j] =
        _prop_d2F[j][i] = NULL;

      // third derivatives
      if (_third_derivatives)
      {
        for (k = j; k < _nargs; ++k)
        {
          if (!_fe_problem.isMatPropRequested(propertyNameThird(_F_name, _arg_names[i], _arg_names[j], _arg_names[k])))
            _prop_d3F[i][j][k] =
            _prop_d3F[k][i][j] =
            _prop_d3F[j][k][i] =
            _prop_d3F[k][j][i] =
            _prop_d3F[j][i][k] =
            _prop_d3F[i][k][j] = NULL;
          else
            needs_third_derivatives = true;
        }

        if (!needs_third_derivatives)
          mooseWarning("This simulation does not actually need the third derivatives of DerivativeBaseMaterial " + _name);
      }
    }
  }
}

// implementing this in the derived class is optional
// (not really, but we'll have to check what is needed for the residuals!)
Real
DerivativeBaseMaterial::computeD3F(unsigned int /*arg1*/, unsigned int /*arg2*/, unsigned int /*arg3*/)
{
  return 0.0;
}

void
DerivativeBaseMaterial::computeProperties()
{
  unsigned int i, j, k;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    // set function value
    if (_prop_F)
      (*_prop_F)[_qp] = computeF();

    for (i = 0; i < _nargs; ++i)
    {
      // set first derivatives
      if (_prop_dF[i])
        (*_prop_dF[i])[_qp] = computeDF(i);

      // second derivatives
      for (j = i; j < _nargs; ++j)
      {
        if (_prop_d2F[i][j])
          (*_prop_d2F[i][j])[_qp] = computeD2F(i, j);

        // third derivatives
        if (_third_derivatives)
        {
          for (k = j; k < _nargs; ++k)
            if (_prop_d3F[i][j][k])
              (*_prop_d3F[i][j][k])[_qp] = computeD3F(i, j, k);
        }
      }
    }
  }
}
