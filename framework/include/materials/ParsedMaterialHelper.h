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

#define usingParsedMaterialHelperMembers(T)                                                        \
  usingFunctionMaterialBaseMembers(T);                                                             \
  usingFunctionParserUtilsMembers(T);                                                              \
  using typename ParsedMaterialHelper<T>::VariableNameMappingMode;                                 \
  using typename ParsedMaterialHelper<T>::MatPropDescriptorList;                                   \
  using ParsedMaterialHelper<T>::functionParse;                                                    \
  using ParsedMaterialHelper<T>::functionsPostParse;                                               \
  using ParsedMaterialHelper<T>::functionsOptimize;                                                \
  using ParsedMaterialHelper<T>::_func_F;                                                          \
  using ParsedMaterialHelper<T>::_symbol_names;                                                    \
  using ParsedMaterialHelper<T>::_mat_prop_descriptors;                                            \
  using ParsedMaterialHelper<T>::_tol;                                                             \
  using ParsedMaterialHelper<T>::_postprocessor_values;                                            \
  using ParsedMaterialHelper<T>::_map_mode

/**
 * Helper class to perform the parsing and optimization of the
 * function expression.
 */
template <bool is_ad>
class ParsedMaterialHelper : public FunctionMaterialBase<is_ad>, public FunctionParserUtils<is_ad>
{
public:
  typedef DerivativeMaterialPropertyNameInterface::SymbolName SymbolName;

  enum class VariableNameMappingMode
  {
    USE_MOOSE_NAMES,
    USE_PARAM_NAMES
  };

  ParsedMaterialHelper(const InputParameters & parameters, VariableNameMappingMode map_mode);

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
  usingFunctionMaterialBaseMembers(is_ad);
  usingFunctionParserUtilsMembers(is_ad);

  void initQpStatefulProperties() override;
  void computeQpProperties() override;

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
};

template <>
void ParsedMaterialHelper<false>::functionsOptimize();

template <>
void ParsedMaterialHelper<true>::functionsOptimize();
