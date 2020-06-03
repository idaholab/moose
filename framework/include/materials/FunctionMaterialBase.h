//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "DerivativeMaterialInterface.h"

#define usingFunctionMaterialBaseMembers(T)                                                        \
  using FunctionMaterialBase<T>::name;                                                             \
  using FunctionMaterialBase<T>::_qp;                                                              \
  using FunctionMaterialBase<T>::_qrule;                                                           \
  using FunctionMaterialBase<T>::_name;                                                            \
  using FunctionMaterialBase<T>::_tid;                                                             \
  using FunctionMaterialBase<T>::_pars;                                                            \
  using FunctionMaterialBase<T>::_material_data_type;                                              \
  using FunctionMaterialBase<T>::_fe_problem;                                                      \
  using FunctionMaterialBase<T>::_args;                                                            \
  using FunctionMaterialBase<T>::_F_name;                                                          \
  using FunctionMaterialBase<T>::_nargs;                                                           \
  using FunctionMaterialBase<T>::_arg_names;                                                       \
  using FunctionMaterialBase<T>::_arg_numbers;                                                     \
  using FunctionMaterialBase<T>::_arg_param_names;                                                 \
  using FunctionMaterialBase<T>::_arg_param_numbers;                                               \
  using FunctionMaterialBase<T>::_arg_constant_defaults;                                           \
  using FunctionMaterialBase<T>::_prop_F

/**
 * Material base class, central to all Materials that provide a Function as a
 * material property value.
 */
template <bool is_ad = false>
class FunctionMaterialBase : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  FunctionMaterialBase(const InputParameters & parameters);

protected:
  /**
   * FunctionMaterialBase keeps an internal list of all the variables the derivatives are taken
   * w.r.t.
   * We provide the MOOSE variable bames in _arg_names, the libMesh variable numbers in
   * _arg_numbers, and the
   * input file parameter names in _arg_param_names. All are indexed by the argument index.
   * This method returns the argument index for a given the libMesh variable number.
   *
   * This mapping is necessary for internal classes which maintain lists of derivatives indexed by
   * argument index
   * and need to pull from those lists from the computeDF, computeD2F, and computeD3F methods, which
   * receive
   * libMesh variable numbers as parameters.
   */
  unsigned int argIndex(unsigned int i_var) const
  {
    const unsigned int idx = libMeshVarNumberRemap(i_var);
    mooseAssert(idx < _arg_index.size() && _arg_numbers[_arg_index[idx]] == i_var,
                "Requesting argIndex() for a derivative w.r.t. a variable not coupled to.");
    return _arg_index[idx];
  }

  /// Coupled variables for function arguments
  std::vector<const GenericVariableValue<is_ad> *> _args;

  /**
   * Name of the function value material property and used as a base name to
   * concatenate the material property names for the derivatives.
   */
  std::string _F_name;

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

  /// Material property to store the function value.
  GenericMaterialProperty<Real, is_ad> * _prop_F;

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
