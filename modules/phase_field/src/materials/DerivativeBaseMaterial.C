#include "DerivativeBaseMaterial.h"

template<>
InputParameters validParams<DerivativeBaseMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Material to provide a function (such as a free energy) and its derivatives w.r.t. the coupled variables");
  params.addParam<std::string>("f_name", "F", "Base name of the free energy function (used to name the material properties)");
  params.addParam<bool>("third_derivatives", true, "Calculate third derivatoves of the free energy");
  return params;
}

DerivativeBaseMaterial::DerivativeBaseMaterial(const std::string & name,
                                               InputParameters parameters) :
    DerivativeMaterialInterface<Material>(name, parameters),
    _F_name(getParam<std::string>("f_name")),
    _third_derivatives(getParam<bool>("third_derivatives")),
    _prop_F(&declareProperty<Real>(_F_name)),
    _number_of_nl_variables(_fe_problem.getNonlinearSystem().nVariables()),
    _arg_index(_number_of_nl_variables)
{
  // loop counters
  unsigned int i, j, k;

  // fetch names and numbers of all coupled variables
  _mapping_is_unique = true;
  for (std::set<std::string>::const_iterator it = _pars.coupledVarsBegin(); it != _pars.coupledVarsEnd(); ++it)
  {
    std::map<std::string, std::vector<MooseVariable *> >::iterator vars = _coupled_vars.find(*it);

    // no MOOSE variable was provided for this coupling, skip derivatives w.r.t. this variable
    if (vars == _coupled_vars.end())
      continue;

    // check if we have a 1:1 mapping between parameters and variables
    if (vars->second.size() != 1)
      _mapping_is_unique = false;

    // iterate over all components
    for (unsigned int j = 0; j < vars->second.size(); ++j)
    {
      // make sure each nonlinear variable is coupled in only once
      if (std::find(_arg_names.begin(), _arg_names.end(), vars->second[j]->name()) != _arg_names.end())
        mooseError("A nonlinear variable can only be coupled in once.");

      // insert the map values
      //unsigned int number = vars->second[j]->number();
      unsigned int number = coupled(*it, j);
      _arg_names.push_back(vars->second[j]->name());
      _arg_numbers.push_back(number);
      _arg_param_names.push_back(*it);

      // populate number -> arg index lookup table skipping aux variables
      if (number < _number_of_nl_variables)
        _arg_index[number] = _args.size();

      // get variable value
      _args.push_back(&coupledValue(*it, j));
    }
  }
  _nargs = _arg_names.size();

  // reserve space for material properties and explicitly initialize to NULL
  _prop_dF.resize(_nargs, NULL);
  _prop_d2F.resize(_nargs);
  _prop_d3F.resize(_nargs);
  for (i = 0; i < _nargs; ++i)
  {
    _prop_d2F[i].resize(_nargs, NULL);

    if (_third_derivatives) {
      _prop_d3F[i].resize(_nargs);

      for (j = 0; j < _nargs; ++j)
        _prop_d3F[i][j].resize(_nargs, NULL);
    }
  }

  // initialize derivatives
  for (i = 0; i < _nargs; ++i)
  {
    // skip all derivatives w.r.t. auxiliary variables
    if (_arg_numbers[i] >= _number_of_nl_variables) continue;

    // first derivatives
    _prop_dF[i] = &declarePropertyDerivative<Real>(_F_name, _arg_names[i]);

    // second derivatives
    for (j = i; j < _nargs; ++j)
    {
      // skip all derivatives w.r.t. auxiliary variables
      if (_arg_numbers[j] >= _number_of_nl_variables) continue;

      _prop_d2F[i][j] =
      _prop_d2F[j][i] = &declarePropertyDerivative<Real>(_F_name, _arg_names[i], _arg_names[j]);

      // third derivatives
      if (_third_derivatives)
      {
        for (k = j; k < _nargs; ++k)
        {
          // skip all derivatives w.r.t. auxiliary variables
          if (_arg_numbers[k] >= _number_of_nl_variables) continue;

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
        (*_prop_dF[i])[_qp] = computeDF(_arg_numbers[i]);

      // second derivatives
      for (j = i; j < _nargs; ++j)
      {
        if (_prop_d2F[i][j])
          (*_prop_d2F[i][j])[_qp] = computeD2F(_arg_numbers[i], _arg_numbers[j]);

        // third derivatives
        if (_third_derivatives)
        {
          for (k = j; k < _nargs; ++k)
            if (_prop_d3F[i][j][k])
              (*_prop_d3F[i][j][k])[_qp] = computeD3F(_arg_numbers[i], _arg_numbers[j], _arg_numbers[k]);
        }
      }
    }
  }
}
