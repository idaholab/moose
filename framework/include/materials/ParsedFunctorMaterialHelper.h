//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctionMaterialBase.h"
#include "FunctionParserUtils.h"
#include "FunctionMaterialPropertyDescriptor.h"
#include "DerivativeMaterialPropertyNameInterface.h"

#include "libmesh/fparser_ad.hh"

#define usingParsedFunctorMaterialHelperMembers(T)                                                        \
  usingFunctionParserUtilsMembers(T);                                                                     \
  using typename ParsedFunctorMaterialHelper<T>::VariableNameMappingMode;                                 \
  using typename ParsedFunctorMaterialHelper<T>::MatPropDescriptorList;                                   \
  using ParsedFunctorMaterialHelper<T>::functionParse;                                                    \
  using ParsedFunctorMaterialHelper<T>::functionsPostParse;                                               \
  using ParsedFunctorMaterialHelper<T>::functionsOptimize;                                                \
  using ParsedFunctorMaterialHelper<T>::_func_F;                                                          \
  using ParsedFunctorMaterialHelper<T>::_symbol_names;                                                    \
  using ParsedFunctorMaterialHelper<T>::_mat_prop_descriptors;                                            \
  using ParsedFunctorMaterialHelper<T>::_tol;                                                             \
  using ParsedFunctorMaterialHelper<T>::_postprocessor_values;                                            \
  using ParsedFunctorMaterialHelper<T>::_map_mode

/**
 * Helper class to perform the parsing and optimization of the
 * function expression.
 */
template <bool is_ad>
class ParsedFunctorMaterialHelper : public FunctorMaterial, public FunctionParserUtils<is_ad>
{
public:
  typedef DerivativeMaterialPropertyNameInterface::SymbolName SymbolName;

  enum class VariableNameMappingMode
  {
    USE_MOOSE_NAMES,
    USE_PARAM_NAMES
  };

  ParsedFunctorMaterialHelper(const InputParameters & parameters, VariableNameMappingMode map_mode);

  static InputParameters validParams();

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
  void functionParse(const std::string & function_expression,
                     const std::vector<std::string> & constant_names,
                     const std::vector<std::string> & constant_expressions,
                     const std::vector<std::string> & mat_prop_names,
                     const std::vector<PostprocessorName> & postprocessor_names,
                     const std::vector<std::string> & tol_names,
                     const std::vector<Real> & tol_values);

protected:
  usingFunctionParserUtilsMembers(is_ad);

  /// Coupled variables for function arguments
  std::vector<const Moose::Functor<ADReal> &> _variables;

  /**
   * Name of the function value material property and used as a base name to
   * concatenate the material property names for the derivatives.
   */
  const std::string _prop_name;

  /// Material property to be computed using the parsed expression
  FunctorMaterialProperty< GenericReal<is_ad> > & _property;

  /// Flag that indicates if exactly one linear variable is coupled per input file coupling parameter
  bool _mapping_is_unique;

  /// Number of coupled arguments.
  unsigned int _nargs;

  /// String vector of all argument names.
  std::vector<std::string> _arg_names;

  /// Vector of all argument MOOSE variable numbers.
  std::vector<unsigned int> _arg_numbers;

  /// String vector of the input file coupling parameter name for each argument.
  std::vector<std::string> _arg_param_names;
  std::vector<int> _arg_param_numbers;

  /// coupled variables with default values
  std::vector<std::string> _arg_constant_defaults;

  // tasks to perform after parsing the primary function
  virtual void functionsPostParse();

  // run FPOptimizer on the parsed function
  virtual void functionsOptimize();

  /// The undiffed free energy function parser object.
  SymFunctionPtr _func_F;

  /**
   * Symbol names used in the expression (depends on the map_mode).
   * We distinguish "symbols" i.e. FParser placeholder names from "variables", which
   * are MOOSE solution objects
   */
  std::vector<SymbolName> _symbol_names;

  /// convenience typedef for the material property descriptors
  typedef std::vector<FunctionMaterialPropertyDescriptor<is_ad>> MatPropDescriptorList;

  /// Material property descriptors (obtained by parsing _mat_prop_expressions)
  MatPropDescriptorList _mat_prop_descriptors;

  /// Tolerance values for all arguments (to protect from log(0)).
  std::vector<Real> _tol;

  /// List of coupled Postprocessors
  std::vector<const PostprocessorValue *> _postprocessor_values;

  /**
   * Flag to indicate if MOOSE nonlinear variable names should be used as FParser variable names.
   * This should be USE_MOOSE_NAMES only for DerivativeParsedMaterial. If set to USE_PARAM_NAMES,
   * this class looks up the input parameter name for each coupled variable and uses it as the
   * FParser parameter name when parsing the FParser expression.
   */
  const VariableNameMappingMode _map_mode;

  /// This is true by default, but can be disabled to make non-existing properties default to zero
  const bool _error_on_missing_material_properties;

private:
  /// map the variable numbers to an even/odd interspersed pattern
  unsigned int libMeshVarNumberRemap(unsigned int var) const
  {
    const int b = static_cast<int>(var);
    return b >= 0 ? b << 1 : (-b << 1) - 1;
  }

  /// helper function for coupling ad/regular variable values
  const GenericVariableValue<is_ad> & coupledGenericValue(const std::string & var_name,
                                                          unsigned int comp = 0);

  /// Vector to look up the internal coupled variable index into _arg_*  through the libMesh variable number
  std::vector<unsigned int> _arg_index;

};

template <>
void ParsedFunctorMaterialHelper<false>::functionsOptimize();

template <>
void ParsedFunctorMaterialHelper<true>::functionsOptimize();
