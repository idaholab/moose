#ifndef PARSEDMATERIALHELPER_H
#define PARSEDMATERIALHELPER_H

#include "Material.h"
#include "libmesh/fparser_ad.hh"

/**
 * Helper class template to perform the parsing and optimization of the
 * function expression.
 */
template<class T>
class ParsedMaterialHelper : public T
{
public:
  enum VariableNameMappingMode {
    USE_MOOSE_NAMES, USE_PARAM_NAMES
  };

  ParsedMaterialHelper(const std::string & name,
                       InputParameters parameters,
                       VariableNameMappingMode map_mode);

  virtual ~ParsedMaterialHelper();

  void functionParse(const std::string & function_expression);
  void functionParse(const std::string & function_expression,
                     const std::vector<std::string> & constant_names,
                     const std::vector<std::string> & constant_expressions);
  void functionParse(const std::string & function_expression,
                     const std::vector<std::string> & constant_names,
                     const std::vector<std::string> & constant_expressions,
                     const std::vector<std::string> & mat_prop_names,
                     const std::vector<std::string> & tol_names,
                     const std::vector<Real> & tol_values);

  static InputParameters validParams();

protected:
  virtual void computeProperties();

  // tasks to perform after parsing the primary function
  virtual void functionsPostParse();

  // run FPOptimizer on the parsed function
  virtual void functionsOptimize();

  /// Shorthand for an autodiff function parser object.
  typedef FunctionParserADBase<Real> ADFunction;

  /// Evaluate FParser object and check EvalError
  Real evaluate(ADFunction *);

  /// The undiffed free energy function parser object.
  ADFunction * _func_F;

  /// Material properties needed by this free energy
  std::vector<MaterialProperty<Real> *> _mat_props;
  unsigned int _nmat_props;

  /// Array to stage the parameters passed to the functions when calling Eval.
  Real * _func_params;

  /// Tolerance values for all arguments (to protect from log(0)).
  std::vector<Real> _tol;

  /// feature flags
  bool _enable_jit;
  bool _disable_fpoptimizer;
  bool _fail_on_evalerror;

  /**
   * Flag to indicate if MOOSE nonlinear variable names should be used as FParser variable names.
   * This should be true only for DerivativeParsedMaterial. If set to false, this class looks up the
   * input parameter name for each coupled variable and uses it as the FParser parameter name when
   * parsing the FParser expression.
   */
  const VariableNameMappingMode _map_mode;

  /// appropriate not a number value to return
  const Real _nan;

  /// table of FParser eval error codes
  static const char * _eval_error_msg[];
};


template<class T>
ParsedMaterialHelper<T>::ParsedMaterialHelper(const std::string & name,
                                           InputParameters parameters,
                                           VariableNameMappingMode map_mode) :
    T(name, parameters),
    _func_F(NULL),
    _nmat_props(0),
#ifdef LIBMESH_HAVE_FPARSER_JIT
    _enable_jit(parameters.get<bool>("enable_jit")),
#else
    _enable_jit(false),
#endif
    _disable_fpoptimizer(parameters.get<bool>("disable_fpoptimizer")),
    _fail_on_evalerror(parameters.get<bool>("fail_on_evalerror")),
    _map_mode(map_mode),
    _nan(std::numeric_limits<Real>::quiet_NaN())
{
}

template<class T>
ParsedMaterialHelper<T>::~ParsedMaterialHelper()
{
  delete[] _func_params;
}

template<class T>
InputParameters
ParsedMaterialHelper<T>::validParams()
{
  InputParameters params = ::validParams<T>();
  params.addClassDescription("Parsed Function Material.");

  // Just in time compilation for the FParser objects
#ifdef LIBMESH_HAVE_FPARSER_JIT
  params.addParam<bool>( "enable_jit", false, "Enable Just In Time compilation of the parsed functions");
#endif
  params.addParam<bool>( "disable_fpoptimizer", false, "Disable the function parser algebraic optimizer");
  params.addParam<bool>( "fail_on_evalerror", false, "Fail fatally if a function evaluation returns an error code (otherwise just pass on NaN)");
  return params;
}

template<class T>
const char * ParsedMaterialHelper<T>::_eval_error_msg[] = {
  "Unknown",
  "Division by zero",
  "Square root of a negative value",
  "Logarithm of negative value",
  "Trigonometric error (asin or acos of illegal value)",
  "Maximum recursion level reached"
};

template<class T>
void ParsedMaterialHelper<T>::functionParse(const std::string & function_expression)
{
  std::vector<std::string> empty_string_vector;
  functionParse(function_expression,
                empty_string_vector, empty_string_vector);
}

template<class T>
void ParsedMaterialHelper<T>::functionParse(const std::string & function_expression,
                                         const std::vector<std::string> & constant_names,
                                         const std::vector<std::string> & constant_expressions)
{
  std::vector<std::string> empty_string_vector;
  std::vector<Real> empty_real_vector;
  functionParse(function_expression, constant_names, constant_expressions,
                empty_string_vector, empty_string_vector, empty_real_vector);
}

template<class T>
void ParsedMaterialHelper<T>::functionParse(const std::string & function_expression,
                                         const std::vector<std::string> & constant_names,
                                         const std::vector<std::string> & constant_expressions,
                                         const std::vector<std::string> & mat_prop_names,
                                         const std::vector<std::string> & tol_names,
                                         const std::vector<Real> & tol_values)
{
  // check number of coupled variables
  if (this->_nargs == 0)
    mooseError("Need at least one coupled variable for ParsedMaterialHelper.");

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
        mooseError("Invalid constant name in ParsedMaterialHelper");

    // build the temporary comnstant expression function
    if (expression->Parse(constant_expressions[i], "") >= 0)
       mooseError(std::string("Invalid constant expression\n" + constant_expressions[i] + "\n in ParsedMaterialHelper. ") + expression->ErrorMsg());

    constant_values[i] = expression->Eval(NULL);

#ifdef DEBUG
    this->_console << "Constant value " << i << ' ' << constant_expressions[i] << " = " << constant_values[i] << std::endl;
#endif

    if (!_func_F->AddConstant(constant_names[i], constant_values[i]))
      mooseError("Invalid constant name in ParsedMaterialHelper");

    delete expression;
  }

  // tolerance vectors
  if (tol_names.size() != tol_values.size())
    mooseError("The parameter vectors tol_names and tol_values must have equal length.");

  // set tolerances
  _tol.resize(this->_nargs);
  for (unsigned int i = 0; i < this->_nargs; ++i)
  {
    _tol[i] = -1.0;

    // for every argument look throug the entire tolerance vector to find a match
    for (unsigned int j = 0; j < tol_names.size(); ++j)
      if (this->_arg_names[i] == tol_names[j])
      {
        _tol[i] = tol_values[j];
        break;
      }
  }

  // build 'variables' argument for fparser
  std::string variables;
  switch (_map_mode)
  {
    case USE_MOOSE_NAMES:
      variables = this->_arg_names[0];
      for (unsigned i = 1; i < this->_nargs; ++i)
        variables += "," + this->_arg_names[i];
      break;

    case USE_PARAM_NAMES:
      // we do not allow vector coupling in this mode
      if (!this->_mapping_is_unique)
        mooseError("Derivative parsed materials must couple exactly one non-linear variable per coupled variable input parameter.");

      variables = this->_arg_param_names[0];
      for (unsigned i = 1; i < this->_nargs; ++i)
        variables += "," + this->_arg_param_names[i];
      break;

    default:
      mooseError("Unnknown variable mapping mode.");
  }

  // get all material properties
  _nmat_props = mat_prop_names.size();
  _mat_props.resize(_nmat_props);
  for (unsigned int i = 0; i < _nmat_props; ++i)
  {
    _mat_props[i] = &(this->template getMaterialProperty<Real>(mat_prop_names[i]));
    variables += "," + mat_prop_names[i];
  }

  // build the base function
  if (_func_F->Parse(function_expression, variables) >= 0)
     mooseError(std::string("Invalid function\n" + function_expression + "\nin ParsedMaterialHelper.\n") + _func_F->ErrorMsg());

  // create parameter passing buffer
  _func_params = new Real[this->_nargs + _nmat_props];

  // perform next steps (either optimize or take derivatives and then optimize)
  functionsPostParse();
}

template<class T>
void ParsedMaterialHelper<T>::functionsPostParse()
{
  functionsOptimize();
}

template<class T>
void ParsedMaterialHelper<T>::functionsOptimize()
{
  // base function
  if (!_disable_fpoptimizer)
    _func_F->Optimize();
  if (_enable_jit && !_func_F->JITCompile())
    mooseWarning("Failed to JIT compile expression, falling back to byte code interpretation.");
}

template<class T>
Real
ParsedMaterialHelper<T>::evaluate(ADFunction * parser)
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
    mooseError("DerivativeParsedMaterial function evaluation encountered an error: "
               << _eval_error_msg[(error_code < 0 || error_code > 5) ? 0 : error_code]);

  return _nan;
}

template<class T>
void
ParsedMaterialHelper<T>::computeProperties()
{
  Real a;

  for (this->_qp = 0; this->_qp < this->_qrule->n_points(); this->_qp++)
  {
    // fill the parameter vector, apply tolerances
    for (unsigned int i = 0; i < this->_nargs; ++i)
    {
      if (_tol[i] < 0.0)
        _func_params[i] = (*this->_args[i])[this->_qp];
      else
      {
        a = (*this->_args[i])[this->_qp];
        _func_params[i] = a < _tol[i] ? _tol[i] : (a > 1.0 - _tol[i] ? 1.0 - _tol[i] : a);
      }
    }

    // insert material property values
    for (unsigned int i = 0; i < _nmat_props; ++i)
      _func_params[i + this->_nargs] = (*_mat_props[i])[this->_qp];

    // TODO: computeQpProperties()

    // set function value
    if (this->_prop_F)
      (*this->_prop_F)[this->_qp] = evaluate(_func_F);
  }
}

#endif //PARSEDMATERIALHELPER_H
