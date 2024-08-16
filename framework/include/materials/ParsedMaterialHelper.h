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

  enum class ExtraSymbols
  {
    x,
    y,
    z,
    t,
    dt
  };

  ParsedMaterialHelper(const InputParameters & parameters, VariableNameMappingMode map_mode);

  static InputParameters validParams();

  /**
   * This method sets up and parses the function string given by the user.
   * @param function_expression Functional expression to parse.
   * Arguments not exposed by this method overload (e.g. post-processors, functors, etc.) are
   * assigned as an empty vector.
   */
  void functionParse(const std::string & function_expression);

  /**
   * This method sets up all variables (e.g. constants)
   * to be used in the function and parses the function string given by the user.
   * Arguments not exposed by this method overload (e.g. post-processors, functors, tolerances) are
   * assigned as an empty vector.
   * @param function_expression Functional expression to parse.
   * @param constant_names Vector of constant names to use.
   * @param constant_expressions Vector of values for the constants in \p constant_names (can be an
   * FParser expression).
   */
  void functionParse(const std::string & function_expression,
                     const std::vector<std::string> & constant_names,
                     const std::vector<std::string> & constant_expressions);

  /**
   * This method sets up all variables (e.g. constants, material properties)
   * to be used in the function and parses the function string given by the user.
   * Arguments not exposed by this method overload (e.g. post-processors, functors) are assigned as
   * an empty vector.
   * @param function_expression Functional expression to parse.
   * @param constant_names Vector of constant names to use.
   * @param constant_expressions Vector of values for the constants in \p constant_names (can be an
   * FParser expression).
   * @param mat_prop_names Vector of material properties used in the parsed function.
   * @param tol_names Vector of variable names to be protected from being 0 or 1 within a tolerance
   * (needed for log(c) and log(1-c) terms).
   * @param tol_values Vector of tolerance values for the variables in \p tol_names .
   */
  void functionParse(const std::string & function_expression,
                     const std::vector<std::string> & constant_names,
                     const std::vector<std::string> & constant_expressions,
                     const std::vector<std::string> & mat_prop_names,
                     const std::vector<std::string> & tol_names,
                     const std::vector<Real> & tol_values);

  /**
   * This method sets up all variables (e.g. constants, material properties, post-processors)
   * to be used in the function and parses the function string given by the user.
   * Arguments not exposed by this method overload (e.g. functors) are assigned as an empty vector.
   * @param function_expression Functional expression to parse.
   * @param constant_names Vector of constant names to use.
   * @param constant_expressions Vector of values for the constants in \p constant_names (can be an
   * FParser expression).
   * @param mat_prop_names Vector of material properties used in the parsed function.
   * @param postprocessor_names Vector of postprocessor names used in the parsed function.
   * @param tol_names Vector of variable names to be protected from being 0 or 1 within a tolerance
   * (needed for log(c) and log(1-c) terms).
   * @param tol_values Vector of tolerance values for the variables in \p tol_names .
   */
  void functionParse(const std::string & function_expression,
                     const std::vector<std::string> & constant_names,
                     const std::vector<std::string> & constant_expressions,
                     const std::vector<std::string> & mat_prop_names,
                     const std::vector<PostprocessorName> & postprocessor_names,
                     const std::vector<std::string> & tol_names,
                     const std::vector<Real> & tol_values);

  /**
   * This method sets up all variables (e.g. constants, material properties, post-processors,
   * functors) to be used in the function and parses the function string given by the user.
   * @param function_expression Functional expression to parse.
   * @param constant_names Vector of constant names to use.
   * @param constant_expressions Vector of values for the constants in \p constant_names (can be an
   * FParser expression).
   * @param mat_prop_names Vector of material properties used in the parsed function.
   * @param postprocessor_names Vector of postprocessor names used in the parsed function.
   * @param tol_names Vector of variable names to be protected from being 0 or 1 within a tolerance
   * (needed for log(c) and log(1-c) terms).
   * @param tol_values Vector of tolerance values for the variables in \p tol_names .
   * @param functor_names vector of constant names to use.
   * @param functor_symbols vector of constant names to use. If this vector is empty, \p
   * functor_names are used as symbol names.
   */
  void functionParse(const std::string & function_expression,
                     const std::vector<std::string> & constant_names,
                     const std::vector<std::string> & constant_expressions,
                     const std::vector<std::string> & mat_prop_names,
                     const std::vector<PostprocessorName> & postprocessor_names,
                     const std::vector<std::string> & tol_names,
                     const std::vector<Real> & tol_values,
                     const std::vector<MooseFunctorName> & functor_names,
                     const std::vector<std::string> & functor_symbols);

protected:
  usingFunctionMaterialBaseMembers(is_ad);
  usingFunctionParserUtilsMembers(is_ad);

  void initQpStatefulProperties() override;
  void computeQpProperties() override;
  virtual void initialSetup() override final;

  /**
   * Populates the given set with names not to be used as user-defined symbol (e.g. for a functor)
   */
  void insertReservedNames(std::set<std::string> & reserved_names);

  // tasks to perform after parsing the primary function
  virtual void functionsPostParse();

  /// The undiffed free energy function parser object.
  SymFunctionPtr _func_F;

  /**
   * Symbol names used in the expression (depends on the map_mode).
   * We distinguish "symbols" i.e. FParser placeholder names from "variables", which
   * are MOOSE solution objects
   */
  std::vector<SymbolName> _symbol_names;

  /// Extra symbols
  const std::vector<ExtraSymbols> _extra_symbols;

  /// Vector of pointers to functors
  std::vector<const Moose::Functor<Real> *> _functors;

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

  /**
   * Vector to hold list of material names that must be updated prior to evaluating current material
   * (for compute = false materials)
   */
  std::vector<MaterialName> _upstream_mat_names;

  /// This is true by default, but can be disabled to make non-existing properties default to zero
  const bool _error_on_missing_material_properties;

  /**
   *  Vector to hold list of materials that must be updated prior to evaluating current material
   * (for compute = false materials)
   */
  std::vector<MaterialBase *> _upstream_mat;
};
