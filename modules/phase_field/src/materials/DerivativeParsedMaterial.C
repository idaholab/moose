#include "DerivativeParsedMaterial.h"

template<>
InputParameters validParams<DerivativeParsedMaterial>()
{
  InputParameters params = validParams<DerivativeBaseMaterial>();
  params.addClassDescription("Parsed Function Material with automatic derivatives.");

  // Constants and their values
  params.addParam<std::vector<std::string> >("constant_names", std::vector<std::string>(), "Vector of constants used in the parsed function (use this for kB etc.)");
  params.addParam<std::vector<std::string> >( "constant_expressions", std::vector<std::string>(), "Vector of values for the constants in constant_names (can be an FParser expression)");

  // Variables with applied tolerances and their tolerance values
  params.addParam<std::vector<std::string> >("tol_names", std::vector<std::string>(), "Vector of variable names to be protected from being 0 or 1 within a tolerance (needed for log(c) and log(1-c) terms)");
  params.addParam<std::vector<Real> >( "tol_values", std::vector<Real>(), "Vector of tolerance values for the variables in tol_names");

  // Material properties
  params.addParam<std::vector<std::string> >(
    "material_property_names", std::vector<std::string>(),
    "Vector of material properties used in the parsed function (these should have a zero gradient!)");

  // Function expression
  params.addRequiredParam<std::string>("function", "FParser function expression for the phase free energy");
  return params;
}

DerivativeParsedMaterial::DerivativeParsedMaterial(const std::string & name,
                                                   InputParameters parameters) :
    DerivativeBaseMaterial(name, parameters)
{
  // check number of coupled variables
  if (_arg_names.size() == 0)
    mooseError("Need at least one couple variable for DerivativeParsedMaterial.");

  // get and check constant vectors
  std::vector<std::string> constant_names = getParam<std::vector<std::string> >("constant_names");
  std::vector<std::string> constant_expressions = getParam<std::vector<std::string> >("constant_expressions");
  unsigned int nconst = constant_expressions.size();
  if (nconst != constant_expressions.size())
    mooseError("The parameter vectors constant_names and constant_values must have equal length.");

  // initialize constants
  ADFunction *expression;
  std::vector<Real> constant_values(nconst);
  for (unsigned int i = 0; i < nconst; ++i)
  {
    expression = new ADFunction();

    // add previously evaluated constants
    for (unsigned int j = 0; j < i; ++j)
      if (!expression->AddConstant(constant_names[j], constant_values[j]))
        mooseError("Invalid constant name in DerivativeParsedMaterial");

    // build the temporary comnstant expression function
    if (expression->Parse(constant_expressions[i], "") >= 0)
       mooseError(std::string("Invalid constant expression\n" + constant_expressions[i] + "\n in DerivativeParsedMaterial. ") + expression->ErrorMsg());

    constant_values[i] = expression->Eval(NULL);

#ifdef DEBUG
    _console << "Constant value " << i << ' ' << constant_expressions[i] << " = " << constant_values[i] << std::endl;
#endif

    if (!_func_F.AddConstant(constant_names[i], constant_values[i]))
      mooseError("Invalid constant name in DerivativeParsedMaterial");

    delete expression;
  }

  // get and check tolerance vectors
  std::vector<std::string> tol_names = getParam<std::vector<std::string> >("tol_names");
  std::vector<Real> tol_values = getParam<std::vector<Real> >("tol_values");

  if (tol_names.size() != tol_values.size())
    mooseError("The parameter vectors tol_names and tol_values must have equal length.");

  // set tolerances
  _tol.resize(_nargs);
  for (unsigned int i = 0; i < _nargs; ++i)
  {
    _tol[i] = -1.0;

    // for every argument look throug the entire tolerance vector to find a match
    for (unsigned int j = 0; j < tol_names.size(); ++j)
      if (_arg_names[i] == tol_names[j])
      {
        _tol[i] = tol_values[j];
        break;
      }
  }

  // build 'variables' argument for fparser
  std::string variables = _arg_names[0];
  for (unsigned i = 1; i < _arg_names.size(); ++i)
    variables += "," + _arg_names[i];

  // get material property names
  std::vector<std::string> mat_prop_names = getParam<std::vector<std::string> >("material_property_names");
  _nmat_props = mat_prop_names.size();

  // get all material properties
  _mat_props.resize(_nmat_props);
  for (unsigned int i = 0; i < _nmat_props; ++i)
  {
    _mat_props[i] = &getMaterialProperty<Real>(mat_prop_names[i]);
    variables += "," + mat_prop_names[i];
  }

  // build the base function
  if (_func_F.Parse(getParam<std::string>("function"), variables) >= 0)
     mooseError(std::string("Invalid function\n" + getParam<std::string>("function") + "\nin DerivativeParsedMaterial.\n") + _func_F.ErrorMsg());

  // Auto-Derivatives
  functionsDerivative();

  // Optimization
  functionsOptimize();

  // create parameter passing buffer
  _func_params = new Real[_nargs + _nmat_props];
}

DerivativeParsedMaterial::~DerivativeParsedMaterial()
{
  delete[] _func_params;
}

void DerivativeParsedMaterial::functionsDerivative()
{
  unsigned int i, j, k;

  // first derivatives
  _func_dF.resize(_nargs);
  _func_d2F.resize(_nargs);
  _func_d3F.resize(_nargs);
  for (i = 0; i < _nargs; ++i)
  {
    _func_dF[i] = new ADFunction(_func_F);
    _func_dF[i]->AutoDiff(_arg_names[i]);

    // second derivatives
    _func_d2F[i].resize(_nargs);
    _func_d3F[i].resize(_nargs);
    for (j = i; j < _nargs; ++j)
    {
      _func_d2F[i][j] = new ADFunction(*_func_dF[i]);
      _func_d2F[i][j]->AutoDiff(_arg_names[j]);

      // third derivatives
      if (_third_derivatives)
      {
        _func_d3F[i][j].resize(_nargs);
        for (k = j; k < _nargs; ++k)
        {
          _func_d3F[i][j][k] = new ADFunction(*_func_d2F[i][j]);
          _func_d3F[i][j][k]->AutoDiff(_arg_names[k]);
        }
      }
    }
  }
}

void DerivativeParsedMaterial::functionsOptimize()
{
  unsigned int i, j, k;

  // base function
  _func_F.Optimize();

  // optimize first derivatives
  for (i = 0; i < _nargs; ++i)
  {
    _func_dF[i]->Optimize();

    // if the derivative vanishes set the function back to NULL
    if (_func_dF[i]->isZero())
    {
      delete _func_dF[i];
      _func_dF[i] = NULL;
    }

    // optimize second derivatives
    for (j = i; j < _nargs; ++j)
    {
      _func_d2F[i][j]->Optimize();

      // if the derivative vanishes set the function back to NULL
      if (_func_d2F[i][j]->isZero())
      {
        delete _func_d2F[i][j];
        _func_d2F[i][j] = NULL;
      }

      // optimize third derivatives
      if (_third_derivatives)
      {
        for (k = j; k < _nargs; ++k)
        {
          _func_d3F[i][j][k]->Optimize();

          // if the derivative vanishes set the function back to NULL
          if (_func_d3F[i][j][k]->isZero())
          {
            delete _func_d3F[i][j][k];
            _func_d3F[i][j][k] = NULL;
          }
        }
      }
    }
  }
}

/// Fm(cmg,cmv,T) takes three arguments
unsigned int
DerivativeParsedMaterial::expectedNumArgs()
{
  // this alwats returns the number of argumens that was passed in
  // i.e. any number of args is accepted.
  return _nargs;
}

/// need to implment these virtuals, although they never get called
Real DerivativeParsedMaterial::computeF() { return 0.0; }
Real DerivativeParsedMaterial::computeDF(unsigned int) { return 0.0; }
Real DerivativeParsedMaterial::computeD2F(unsigned int, unsigned int) { return 0.0; }

void
DerivativeParsedMaterial::computeProperties()
{
  unsigned int i, j, k;
  Real a;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    // fill the parameter vector, apply tolerances
    for (i = 0; i < _nargs; ++i)
    {
      if (_tol[i] < 0.0)
        _func_params[i] = (*_args[i])[_qp];
      else
      {
        a = (*_args[i])[_qp];
        _func_params[i] = a < _tol[i] ? _tol[i] : (a > 1.0 - _tol[i] ? 1.0 - _tol[i] : a);
      }
    }

    // insert material property values
    for (i = 0; i < _nmat_props; ++i)
      _func_params[i + _nargs] = (*_mat_props[i])[_qp];

    // set function value
    if (_prop_F)
      (*_prop_F)[_qp] = _func_F.Eval(_func_params);

    for (i = 0; i < _nargs; ++i)
    {
      if (_prop_dF[i])
        (*_prop_dF[i])[_qp] = _func_dF[i]==NULL ? 0.0 : _func_dF[i]->Eval(_func_params);

      // second derivatives
      for (j = i; j < _nargs; ++j)
      {
        if (_prop_d2F[i][j])
          (*_prop_d2F[i][j])[_qp] = _func_d2F[i][j]==NULL ? 0.0 : _func_d2F[i][j]->Eval(_func_params);

        // third derivatives
        if (_third_derivatives)
          for (k = j; k < _nargs; ++k)
            if (_prop_d3F[i][j][k])
              (*_prop_d3F[i][j][k])[_qp] = _func_d3F[i][j][k]==NULL ? 0.0 : _func_d3F[i][j][k]->Eval(_func_params);
      }
    }
  }
}
