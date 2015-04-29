#include "ParsedMaterialHelper.h"

template<>
InputParameters validParams<ParsedMaterialHelper>()
{
  InputParameters params = validParams<FunctionMaterialBase>();
  params += validParams<FunctionParserUtils>();
  params.addClassDescription("Parsed Function Material.");
  return params;
}

ParsedMaterialHelper::ParsedMaterialHelper(const std::string & name,
                                           InputParameters parameters,
                                           VariableNameMappingMode map_mode) :
    FunctionMaterialBase(name, parameters),
    FunctionParserUtils(name, parameters),
    _func_F(NULL),
    _mat_prop_descriptors(0),
    _tol(0),
    _map_mode(map_mode)
{
}

ParsedMaterialHelper::~ParsedMaterialHelper()
{
  delete _func_F;
}

void
ParsedMaterialHelper::functionParse(const std::string & function_expression)
{
  std::vector<std::string> empty_string_vector;
  functionParse(function_expression,
                empty_string_vector, empty_string_vector);
}

void
ParsedMaterialHelper::functionParse(const std::string & function_expression,
                                    const std::vector<std::string> & constant_names,
                                    const std::vector<std::string> & constant_expressions)
{
  std::vector<std::string> empty_string_vector;
  std::vector<Real> empty_real_vector;
  functionParse(function_expression, constant_names, constant_expressions,
                empty_string_vector, empty_string_vector, empty_real_vector);
}

void
ParsedMaterialHelper::functionParse(const std::string & function_expression,
                                    const std::vector<std::string> & constant_names,
                                    const std::vector<std::string> & constant_expressions,
                                    const std::vector<std::string> & mat_prop_expressions,
                                    const std::vector<std::string> & tol_names,
                                    const std::vector<Real> & tol_values)
{
  // build base function object
  _func_F =  new ADFunction();

  // initialize constants
  addFParserConstants(_func_F, constant_names, constant_expressions);

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
  switch (_map_mode)
  {
    case USE_MOOSE_NAMES:
      for (unsigned i = 0; i < _nargs; ++i)
        variables += "," + _arg_names[i];
      break;

    case USE_PARAM_NAMES:
      // we do not allow vector coupling in this mode
      if (!_mapping_is_unique)
        mooseError("Derivative parsed materials must couple exactly one non-linear variable per coupled variable input parameter.");

      for (unsigned i = 0; i < _nargs; ++i)
        variables += "," + _arg_param_names[i];
      break;

    default:
      mooseError("Unnknown variable mapping mode.");
  }

  // get all material properties
  unsigned int nmat_props = mat_prop_expressions.size();
  _mat_prop_descriptors.resize(nmat_props);
  for (unsigned int i = 0; i < nmat_props; ++i)
  {
    // parse the material property parameter entry into a FunctionMaterialPropertyDescriptor
    _mat_prop_descriptors[i] = FunctionMaterialPropertyDescriptor(mat_prop_expressions[i], this);

    // get the fparser symbol name for the new material property
    variables += "," + _mat_prop_descriptors[i].getSymbolName();
  }

  // erase leading comma
  variables.erase(0,1);

  // build the base function
  if (_func_F->Parse(function_expression, variables) >= 0)
     mooseError("Invalid function\n" << function_expression << '\n' <<
                variables << "\nin ParsedMaterialHelper.\n" << _func_F->ErrorMsg());

  // create parameter passing buffer
  _func_params.resize(_nargs + nmat_props);

  // perform next steps (either optimize or take derivatives and then optimize)
  functionsPostParse();
}

void
ParsedMaterialHelper::functionsPostParse()
{
  functionsOptimize();
}

void
ParsedMaterialHelper::functionsOptimize()
{
  // base function
  if (!_disable_fpoptimizer)
    _func_F->Optimize();
  if (_enable_jit && !_func_F->JITCompile())
    mooseWarning("Failed to JIT compile expression, falling back to byte code interpretation.");
}

void
ParsedMaterialHelper::computeProperties()
{
  Real a;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    // fill the parameter vector, apply tolerances
    for (unsigned int i = 0; i < _nargs; ++i)
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
    unsigned int nmat_props = _mat_prop_descriptors.size();
    for (unsigned int i = 0; i < nmat_props; ++i)
      _func_params[i + _nargs] = _mat_prop_descriptors[i].value()[_qp];

    // TODO: computeQpProperties()

    // set function value
    if (_prop_F)
      (*_prop_F)[_qp] = evaluate(_func_F);
  }
}



ParsedMaterialHelper::FunctionMaterialPropertyDescriptor::FunctionMaterialPropertyDescriptor(const std::string & expression, ParsedMaterialHelper * parent) :
    _dependent_vars(),
    _derivative_vars(),
    _parent_material(parent)
{
  size_t define = expression.find_last_of(":=");

  // expression contains a ':='
  if (define != std::string::npos)
  {
    // section before ':=' is the name used in the function expression
    _fparser_name = expression.substr(0, define);

    // parse right hand side
    parseDerivative(expression.substr(define+2));
  }
  else
  {
    // parse entire expression and use natural material property base name
    // for D(x(t),t,t) this would simply be 'x'!
    parseDerivative(expression);
    _fparser_name = _base_name;
  }

  // get the material property reference
  _value = &(parent->getMaterialProperty<Real>(getPropertyName()));
}

ParsedMaterialHelper::FunctionMaterialPropertyDescriptor::FunctionMaterialPropertyDescriptor() :
    _value(NULL),
    _parent_material(NULL)
{
}

ParsedMaterialHelper::FunctionMaterialPropertyDescriptor::FunctionMaterialPropertyDescriptor(const FunctionMaterialPropertyDescriptor & rhs) :
    _fparser_name(rhs._fparser_name),
    _base_name(rhs._base_name),
    _dependent_vars(rhs._dependent_vars),
    _derivative_vars(rhs._derivative_vars),
    _value(rhs._value),
    _parent_material(rhs._parent_material)
{
}

void
ParsedMaterialHelper::FunctionMaterialPropertyDescriptor::parseDerivative(const std::string & expression)
{
  size_t open  = expression.find_first_of("[");
  size_t close = expression.find_last_of("]");

  if (open == std::string::npos && close == std::string::npos)
  {
    // no derivative requested
    parseDependentVariables(expression);
    return;
  }
  else if (open != std::string::npos && close != std::string::npos && expression.substr(0, open) == "D")
  {
    // parse argument list 0 is the function and 1,.. ar the variable to take the derivative w.r.t.
    MooseUtils::tokenize(expression.substr(open + 1, close - open - 1), _derivative_vars, 0, ",");

    // check for empty [] brackets
    if (_derivative_vars.size() > 0)
    {
      // parse argument zero of D[] as the function material property
      parseDependentVariables(_derivative_vars[0]);

      // remove function from the _derivative_vars vector
      _derivative_vars.erase(_derivative_vars.begin());
      return;
    }
  }

  mooseError("Malformed material_properties expression '" << expression << "'");
}

void
ParsedMaterialHelper::FunctionMaterialPropertyDescriptor::parseDependentVariables(const std::string & expression)
{
  size_t open  = expression.find_first_of("(");
  size_t close = expression.find_last_of(")");

  if (open == std::string::npos && close == std::string::npos)
  {
    // material property name without arguments
    _fparser_name = _base_name = expression;
  }
  else if (open != std::string::npos && close != std::string::npos)
  {
    // take material property name before bracket
    _base_name = expression.substr(0, open);

    // parse argument list
    MooseUtils::tokenize(expression.substr(open + 1, close - open - 1), _dependent_vars, 0, ",");

    // cremove duplicates from dependent variable list
    std::sort(_dependent_vars.begin(), _dependent_vars.end());
    _dependent_vars.erase(std::unique(_dependent_vars.begin(), _dependent_vars.end()), _dependent_vars.end());
  }
  else
    mooseError("Malformed material_properties expression '" << expression << "'");
}
