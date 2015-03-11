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

  /// Material properties needed by this free energy
  std::vector<MaterialProperty<Real> *> _mat_props;
  unsigned int _nmat_props;

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
    _nmat_props(0),
    _map_mode(map_mode)
{
}

template<class T>
ParsedMaterialHelper<T>::~ParsedMaterialHelper()
{
  delete _func_F;
  delete[] _func_params;
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
