/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef PARSEDMATERIALHELPER_H
#define PARSEDMATERIALHELPER_H

#include "Material.h"
#include "FunctionParserUtils.h"
#include "libmesh/fparser_ad.hh"

/**
 * Helper class template to perform the parsing and optimization of the
 * function expression.
 * This should be templated on FunctionMaterialBase or DerivativeFunctionMaterialBase.
 */
template<class T>
class ParsedMaterialHelper :
  public T,
  public FunctionParserUtils
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

  /// The undiffed free energy function parser object.
  ADFunction * _func_F;

  /*
   * Material properties get fully described using this structure, including their dependent
   * variables and derivation state.
   */
  class FunctionMaterialPropertyDescriptor;

  /// Material property descriptors (obtained by parsing _mat_prop_expressions)
  std::vector<FunctionMaterialPropertyDescriptor> _mat_prop_descriptors;

  /// Tolerance values for all arguments (to protect from log(0)).
  std::vector<Real> _tol;

  /**
   * Flag to indicate if MOOSE nonlinear variable names should be used as FParser variable names.
   * This should be true only for DerivativeParsedMaterial. If set to false, this class looks up the
   * input parameter name for each coupled variable and uses it as the FParser parameter name when
   * parsing the FParser expression.
   */
  const VariableNameMappingMode _map_mode;
};


template<class T>
ParsedMaterialHelper<T>::ParsedMaterialHelper(const std::string & name,
                                              InputParameters parameters,
                                              VariableNameMappingMode map_mode) :
    T(name, parameters),
    FunctionParserUtils(name, parameters),
    _func_F(NULL),
    _mat_prop_descriptors(0),
    _tol(0),
    _map_mode(map_mode)
{
}

template<class T>
ParsedMaterialHelper<T>::~ParsedMaterialHelper()
{
  delete _func_F;
}

template<class T>
InputParameters
ParsedMaterialHelper<T>::validParams()
{
  InputParameters params = ::validParams<T>();
  params += ::validParams<FunctionParserUtils>();
  params.addClassDescription("Parsed Function Material.");
  return params;
}

template<class T>
void
ParsedMaterialHelper<T>::functionParse(const std::string & function_expression)
{
  std::vector<std::string> empty_string_vector;
  functionParse(function_expression,
                empty_string_vector, empty_string_vector);
}

template<class T>
void
ParsedMaterialHelper<T>::functionParse(const std::string & function_expression,
                                       const std::vector<std::string> & constant_names,
                                       const std::vector<std::string> & constant_expressions)
{
  std::vector<std::string> empty_string_vector;
  std::vector<Real> empty_real_vector;
  functionParse(function_expression, constant_names, constant_expressions,
                empty_string_vector, empty_string_vector, empty_real_vector);
}

template<class T>
void
ParsedMaterialHelper<T>::functionParse(const std::string & function_expression,
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
      for (unsigned i = 0; i < this->_nargs; ++i)
        variables += "," + this->_arg_names[i];
      break;

    case USE_PARAM_NAMES:
      // we do not allow vector coupling in this mode
      if (!this->_mapping_is_unique)
        mooseError("Derivative parsed materials must couple exactly one non-linear variable per coupled variable input parameter.");

      for (unsigned i = 0; i < this->_nargs; ++i)
        variables += "," + this->_arg_param_names[i];
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

    // get the fparser symbol name for teh new material property
    variables += "," + _mat_prop_descriptors[i].getSymbolName();
  }

  // erase leading comma
  variables.erase(0,1);

  // build the base function
  if (_func_F->Parse(function_expression, variables) >= 0)
     mooseError("Invalid function\n" << function_expression << '\n' << variables << "\nin ParsedMaterialHelper.\n" << _func_F->ErrorMsg());

  // create parameter passing buffer
  _func_params.resize(this->_nargs + nmat_props);

  // perform next steps (either optimize or take derivatives and then optimize)
  functionsPostParse();
}

template<class T>
void
ParsedMaterialHelper<T>::functionsPostParse()
{
  functionsOptimize();
}

template<class T>
void
ParsedMaterialHelper<T>::functionsOptimize()
{
  // base function
  if (!_disable_fpoptimizer)
    _func_F->Optimize();
  if (_enable_jit && !_func_F->JITCompile())
    mooseWarning("Failed to JIT compile expression, falling back to byte code interpretation.");
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
    unsigned int nmat_props = _mat_prop_descriptors.size();
    for (unsigned int i = 0; i < nmat_props; ++i)
      _func_params[i + this->_nargs] = _mat_prop_descriptors[i].value()[this->_qp];

    // TODO: computeQpProperties()

    // set function value
    if (this->_prop_F)
      (*this->_prop_F)[this->_qp] = evaluate(_func_F);
  }
}

template<class T>
class ParsedMaterialHelper<T>::FunctionMaterialPropertyDescriptor
{
public:
  /*
   * The descriptor is constructed with an expression that describes the
   * material property.
   * Examples:
   *   'F'               A material property called 'F' with no declared variable
   *                     dependencies (i.e. vanishing derivatives)
   *   'F(c,phi)'        A material property called 'F' with declared dependence
   *                     on 'c' and 'phi' (uses DerivativeFunctionMaterial rules to
   *                     look up the derivatives)
   *   'a:=D[x(t),t,t]'  The second time derivative of the t-dependent material property 'x'
   *                     which will be referred to as 'a' in the function expression.
   */
  FunctionMaterialPropertyDescriptor(const std::string &, T *);

  /// default constructor
  FunctionMaterialPropertyDescriptor();

  /// copy constructor
  FunctionMaterialPropertyDescriptor(const FunctionMaterialPropertyDescriptor &);

  /// get the fparser symbol name
  const std::string & getSymbolName() const { return _fparser_name; };

  /// get the property name
  const std::string getPropertyName() const
  {
    mooseAssert( _parent_material != NULL, "_parent_material pointer is NULL" );
    return _parent_material->propertyName(_base_name, _derivative_vars);
  };

  /// get the property reference
  const MaterialProperty<Real> & value() const
  {
    mooseAssert( _value != NULL, "_value pointer is NULL" );
    return *_value;
  }

private:
  void parseDerivative(const std::string &);
  void parseDependentVariables(const std::string &);

  /// name used in function expression
  std::string _fparser_name;

  /// function material property base name
  std::string _base_name;

  std::vector<std::string> _dependent_vars;
  std::vector<std::string> _derivative_vars;

  /// material property value
  MaterialProperty<Real> * _value;

  /// parent material class
  T * _parent_material;
};

template<class T>
ParsedMaterialHelper<T>::FunctionMaterialPropertyDescriptor::FunctionMaterialPropertyDescriptor(const std::string & expression, T * parent) :
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
  _value = &(parent->template getMaterialProperty<Real>(getPropertyName()));
}

template<class T>
ParsedMaterialHelper<T>::FunctionMaterialPropertyDescriptor::FunctionMaterialPropertyDescriptor() :
    _value(NULL),
    _parent_material(NULL)
{
}

template<class T>
ParsedMaterialHelper<T>::FunctionMaterialPropertyDescriptor::FunctionMaterialPropertyDescriptor(const FunctionMaterialPropertyDescriptor & rhs) :
    _fparser_name(rhs._fparser_name),
    _base_name(rhs._base_name),
    _dependent_vars(rhs._dependent_vars),
    _derivative_vars(rhs._derivative_vars),
    _value(rhs._value),
    _parent_material(rhs._parent_material)
{
}

template<class T>
void
ParsedMaterialHelper<T>::FunctionMaterialPropertyDescriptor::parseDerivative(const std::string & expression)
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

template<class T>
void
ParsedMaterialHelper<T>::FunctionMaterialPropertyDescriptor::parseDependentVariables(const std::string & expression)
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

#endif //PARSEDMATERIALHELPER_H
