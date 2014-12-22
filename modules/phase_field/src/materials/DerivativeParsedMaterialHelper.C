#include "DerivativeParsedMaterialHelper.h"

template<>
InputParameters validParams<DerivativeParsedMaterialHelper>()
{
  InputParameters params = validParams<DerivativeBaseMaterial>();
  params.addClassDescription("Parsed Function Material with automatic derivatives.");

  // Just in time compilation for the FParser objects
#ifdef LIBMESH_HAVE_FPARSER_JIT
  params.addParam<bool>( "enable_jit", false, "Enable Just In Time compilation of the parsed functions");
#endif
  params.addParam<bool>( "disable_fpoptimizer", false, "Disable the function parser algebraic optimizer");
  params.addParam<bool>( "fail_on_evalerror", false, "Fail fatally if a function evaluation returns an error code (otherwise just pass on NaN)");
  return params;
}

const char * DerivativeParsedMaterialHelper::_eval_error_msg[] = {
  "Unknown",
  "Division by zero",
  "Square root of a negative value",
  "Logarithm of negative value",
  "Trigonometric error (asin or acos of illegal value)",
  "Maximum recursion level reached"
};

DerivativeParsedMaterialHelper::DerivativeParsedMaterialHelper(const std::string & name,
                                                               InputParameters parameters,
                                                               bool use_variable_names_verbatim) :
    DerivativeBaseMaterial(name, parameters),
    _func_F(NULL),
    _nmat_props(0),
    _enable_jit(isParamValid("enable_jit") && getParam<bool>("enable_jit")),
    _disable_fpoptimizer(getParam<bool>("disable_fpoptimizer")),
    _fail_on_evalerror(getParam<bool>("fail_on_evalerror")),
    _use_variable_names_verbatim(use_variable_names_verbatim),
    _nan(std::numeric_limits<Real>::quiet_NaN())
{
}

DerivativeParsedMaterialHelper::~DerivativeParsedMaterialHelper()
{
  delete[] _func_params;
}

void DerivativeParsedMaterialHelper::functionParse(const std::string & function_expression)
{
  std::vector<std::string> empty_string_vector;
  functionParse(function_expression,
                empty_string_vector, empty_string_vector);
}

void DerivativeParsedMaterialHelper::functionParse(const std::string & function_expression,
                                                   const std::vector<std::string> & constant_names,
                                                   const std::vector<std::string> & constant_expressions)
{
  std::vector<std::string> empty_string_vector;
  std::vector<Real> empty_real_vector;
  functionParse(function_expression, constant_names, constant_expressions,
                empty_string_vector, empty_string_vector, empty_real_vector);
}

void DerivativeParsedMaterialHelper::functionParse(const std::string & function_expression,
                                                   const std::vector<std::string> & constant_names,
                                                   const std::vector<std::string> & constant_expressions,
                                                   const std::vector<std::string> & mat_prop_names,
                                                   const std::vector<std::string> & tol_names,
                                                   const std::vector<Real> & tol_values)
{
  // check number of coupled variables
  if (_nargs == 0)
    mooseError("Need at least one coupled variable for DerivativeParsedMaterialHelper.");

  // check constant vectors
  unsigned int nconst = constant_expressions.size();
  if (nconst != constant_expressions.size())
    mooseError("The parameter vectors constant_names and constant_values must have equal length.");

  // build base function object
  _func_F =  new ADFunction();

  // initialize constants
  ADFunction *expression;
  std::vector<Real> constant_values(nconst);
  for (unsigned int i = 0; i < nconst; ++i)
  {
    expression = new ADFunction();

    // add previously evaluated constants
    for (unsigned int j = 0; j < i; ++j)
      if (!expression->AddConstant(constant_names[j], constant_values[j]))
        mooseError("Invalid constant name in DerivativeParsedMaterialHelper");

    // build the temporary comnstant expression function
    if (expression->Parse(constant_expressions[i], "") >= 0)
       mooseError(std::string("Invalid constant expression\n" + constant_expressions[i] + "\n in DerivativeParsedMaterialHelper. ") + expression->ErrorMsg());

    constant_values[i] = expression->Eval(NULL);

#ifdef DEBUG
    _console << "Constant value " << i << ' ' << constant_expressions[i] << " = " << constant_values[i] << std::endl;
#endif

    if (!_func_F->AddConstant(constant_names[i], constant_values[i]))
      mooseError("Invalid constant name in DerivativeParsedMaterialHelper");

    delete expression;
  }

  // tolerance vectors
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
  std::string variables;
  if (_use_variable_names_verbatim)
  {
    variables = _arg_names[0];
    for (unsigned i = 1; i < _nargs; ++i)
      variables += "," + _arg_names[i];
  }
  else
  {
    // vector of the input file parameter names for each coupled variable in _arg_names
    std::vector<std::string> arg_param_names(_nargs);

    // enumerate all coupled variables
    for (std::set<std::string>::const_iterator it = _pars.coupledVarsBegin(); it != _pars.coupledVarsEnd(); ++it)
    {
      std::map<std::string, std::vector<MooseVariable *> >::iterator vars = _coupled_vars.find(*it);

      // no MOOSE variable was provided for this coupling
      if (vars == _coupled_vars.end())
        mooseError("Derivative parsed materials do not work with optional/default coupling yet. Please use addRequiredCoupledVar and provide coupling for all variables.");

      // we do not allow vector coupling in this mode
      if (vars->second.size() != 1)
        mooseError("Derivative parsed materials mut couple exactly one non-linear variable per couple variable input parameter.");

      // find the name of this MOOSE var in _arg_names
      std::vector<std::string>::iterator pos = std::find(_arg_names.begin(), _arg_names.end(), vars->second[0]->name());

      // check that we get a valid position
      if (pos == _arg_names.end())
        mooseError("Coupled variable not found.");

      // make sure the mapping is unique, i.e. the user must not couple in the same variable multiple times
      unsigned int index = pos - _arg_names.begin();
      if (arg_param_names[index] != "")
        mooseError("Coupling needs to be unique. You cannot couple the same non-linear variable multiple times.");

      // insert the map value
      arg_param_names[index] = *it;
    }

    // make sure we got a map value for each coupled variable
    for (unsigned i = 0; i < _nargs; ++i)
      if (arg_param_names[i] == "")
        mooseError("No parameter name found for coupled varaiable " << _arg_names[i]);

    variables = arg_param_names[0];
    for (unsigned i = 1; i < _nargs; ++i)
      variables += "," + arg_param_names[i];
  }

  // get all material properties
  _nmat_props = mat_prop_names.size();
  _mat_props.resize(_nmat_props);
  for (unsigned int i = 0; i < _nmat_props; ++i)
  {
    _mat_props[i] = &getMaterialProperty<Real>(mat_prop_names[i]);
    variables += "," + mat_prop_names[i];
  }

  // build the base function
  if (_func_F->Parse(function_expression, variables) >= 0)
     mooseError(std::string("Invalid function\n" + function_expression + "\nin DerivativeParsedMaterialHelper.\n") + _func_F->ErrorMsg());

  // Auto-Derivatives
  functionsDerivative();

  // Optimization
  functionsOptimize();

  // create parameter passing buffer
  _func_params = new Real[_nargs + _nmat_props];
}

void DerivativeParsedMaterialHelper::functionsDerivative()
{
  unsigned int i, j, k;

  // first derivatives
  _func_dF.resize(_nargs);
  _func_d2F.resize(_nargs);
  _func_d3F.resize(_nargs);
  for (i = 0; i < _nargs; ++i)
  {
    _func_dF[i] = new ADFunction(*_func_F);
    if (_func_dF[i]->AutoDiff(_arg_names[i]) != -1)
      mooseError("Failed to take first derivative.");

    // second derivatives
    _func_d2F[i].resize(_nargs);
    _func_d3F[i].resize(_nargs);
    for (j = i; j < _nargs; ++j)
    {
      _func_d2F[i][j] = new ADFunction(*_func_dF[i]);
      if (_func_d2F[i][j]->AutoDiff(_arg_names[j]) != -1)
        mooseError("Failed to take second derivative.");

      // third derivatives
      if (_third_derivatives)
      {
        _func_d3F[i][j].resize(_nargs);
        for (k = j; k < _nargs; ++k)
        {
          _func_d3F[i][j][k] = new ADFunction(*_func_d2F[i][j]);
          if (_func_d3F[i][j][k]->AutoDiff(_arg_names[k]) != -1)
            mooseError("Failed to take third derivative.");
        }
      }
    }
  }
}

void DerivativeParsedMaterialHelper::functionsOptimize()
{
  unsigned int i, j, k;

  // base function
  if (!_disable_fpoptimizer)
    _func_F->Optimize();
  if (_enable_jit && !_func_F->JITCompile())
    mooseWarning("Failed to JIT compile expression, falling back to byte code interpretation.");

  // optimize first derivatives
  for (i = 0; i < _nargs; ++i)
  {
    if (!_disable_fpoptimizer)
      _func_dF[i]->Optimize();
    if (_enable_jit && !_func_dF[i]->JITCompile())
      mooseWarning("Failed to JIT compile expression, falling back to byte code interpretation.");

    // if the derivative vanishes set the function back to NULL
    if (_func_dF[i]->isZero())
    {
      delete _func_dF[i];
      _func_dF[i] = NULL;
    }

    // optimize second derivatives
    for (j = i; j < _nargs; ++j)
    {
      if (!_disable_fpoptimizer)
        _func_d2F[i][j]->Optimize();
      if (_enable_jit && !_func_d2F[i][j]->JITCompile())
        mooseWarning("Failed to JIT compile expression, falling back to byte code interpretation.");

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
          if (!_disable_fpoptimizer)
            _func_d3F[i][j][k]->Optimize();
          if (_enable_jit && !_func_d3F[i][j][k]->JITCompile())
            mooseWarning("Failed to JIT compile expression, falling back to byte code interpretation.");

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

/// need to implment these virtuals, although they never get called
Real DerivativeParsedMaterialHelper::computeF() { return 0.0; }
Real DerivativeParsedMaterialHelper::computeDF(unsigned int) { return 0.0; }
Real DerivativeParsedMaterialHelper::computeD2F(unsigned int, unsigned int) { return 0.0; }

Real
DerivativeParsedMaterialHelper::evaluate(ADFunction * parser)
{
  // null pointer is a shortcut for vanishing derivatives, see functionsOptimize()
  if (parser == NULL) return 0.0;

  // evaluate expression
  Real result = parser->Eval(_func_params);

  // fetch fparser evaluation error
  int error_code = parser->EvalError();

  // no error
  if (error_code == 0)
    return result;

  // hard fail or return not a number
  if (_fail_on_evalerror)
    mooseError("DerivativeParsedMaterial function evaluation encountered an error: " << _eval_error_msg[(error_code < 0 || error_code > 5) ? 0 : error_code]);

  return _nan;
}

void
DerivativeParsedMaterialHelper::computeProperties()
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
      (*_prop_F)[_qp] = evaluate(_func_F);

    for (i = 0; i < _nargs; ++i)
    {
      if (_prop_dF[i])
        (*_prop_dF[i])[_qp] = evaluate(_func_dF[i]);

      // second derivatives
      for (j = i; j < _nargs; ++j)
      {
        if (_prop_d2F[i][j])
          (*_prop_d2F[i][j])[_qp] = evaluate(_func_d2F[i][j]);

        // third derivatives
        if (_third_derivatives)
          for (k = j; k < _nargs; ++k)
            if (_prop_d3F[i][j][k])
              (*_prop_d3F[i][j][k])[_qp] = evaluate(_func_d3F[i][j][k]);
      }
    }
  }
}
