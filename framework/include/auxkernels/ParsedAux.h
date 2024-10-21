//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include "FunctionParserUtils.h"

/**
 * AuxKernel that evaluates a parsed function expression
 */
class ParsedAux : public AuxKernel, public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();

  ParsedAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Function to validate the symbols in _functor_symbols
  void validateFunctorSymbols();

  /// Function to validate the names in _functor_names
  void validateFunctorNames();

  /**
   * Function to ensure vector entries (names) do not overlap with xyzt or coupled variable names.
   * @param names_vec Vector containing names to compare to xyzt and coupled variables names.
   * @param param_name Name of the parameter corresponding to names_vec. This will be the paremeter
   * errored on, if applicable.
   */
  template <typename T>
  void validateGenericVectorNames(const std::vector<T> & names_vec, const std::string & param_name);

  /// function expression
  std::string _function;

  /// coupled variables
  const unsigned int _nargs;
  const std::vector<const VariableValue *> _args;

  /// material properties
  const std::vector<MaterialPropertyName> & _matprop_names;
  const std::vector<MaterialPropertyName> & _ad_matprop_names;
  const unsigned int _n_matprops;
  const unsigned int _n_ad_matprops;
  std::vector<const MaterialProperty<Real> *> _matprops;
  std::vector<const ADMaterialProperty<Real> *> _ad_matprops;

  /// import coordinates and time
  const bool _use_xyzt;

  /// coordinate and time variable names
  const std::vector<std::string> _xyzt;

  /// function parser object for the resudual and on-diagonal Jacobian
  SymFunctionPtr _func_F;

  usingFunctionParserUtilsMembers(false);

  /// Functors to use in the parsed expression
  const std::vector<MooseFunctorName> & _functor_names;

  /// Number of functors
  const unsigned int _n_functors;

  /// Symbolic name to use for each functor
  const std::vector<std::string> _functor_symbols;

  /// Vector of pointers to functors
  std::vector<const Moose::Functor<Real> *> _functors;

  /// Vector of coupled variable names
  std::vector<std::string> _coupled_variable_names;
};

template <typename T>
void
ParsedAux::validateGenericVectorNames(const std::vector<T> & names_vec,
                                      const std::string & param_name)
{
  for (const auto & name : names_vec)
  {
    // Make sure symbol is not x, y, z, or t
    if (_use_xyzt && (std::find(_xyzt.begin(), _xyzt.end(), name) != _xyzt.end()))
      paramError(
          param_name,
          "x, y, z, and t cannot be used in '" + param_name + "' when use_xyzt=true." +
              (param_name == "functor_names" ? " Use 'functor_symbols' to disambiguate." : ""));
    // Make sure symbol is not a coupled variable name
    if (_coupled_variable_names.size() &&
        (std::find(_coupled_variable_names.begin(), _coupled_variable_names.end(), name) !=
         _coupled_variable_names.end()))
      paramError(
          param_name,
          "Values in '" + param_name + "' cannot overlap with coupled variable names." +
              (param_name == "functor_names" ? " Use 'functor_symbols' to disambiguate." : ""));
  }
}
