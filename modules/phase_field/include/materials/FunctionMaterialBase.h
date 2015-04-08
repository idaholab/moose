/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef FUNCTIONMATERIALBASE_H
#define FUNCTIONMATERIALBASE_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations
class FunctionMaterialBase;

template<>
InputParameters validParams<FunctionMaterialBase>();

/**
 * %Material base class central for all Materials that provide a Function as a
 * material property value.
 */
class FunctionMaterialBase : public DerivativeMaterialInterface<Material>
{
public:
  FunctionMaterialBase(const std::string & name, InputParameters parameters);

protected:

  /**
   * FunctionMaterialBase keeps an internal list of all the variables the derivatives are taken w.r.t.
   * We provide the MOOSE variable bames in _arg_names, the libMesh variable numbers in _arg_numbers, and the
   * input file parameter names in _arg_param_names. All are indexed by the argument index.
   * This method returns the argument index for a given the libMesh variable number.
   *
   * This mapping is necessary for internal classes which maintain lists of derivatives indexed by argument index
   * and need to pull from those lists from the computeDF, computeD2F, and computeD3F methods, which receive
   * libMesh variable numbers as parameters.
   */
  unsigned int argIndex(unsigned int i_var) const
  {
    mooseAssert(i_var < _number_of_nl_variables, "Requesting argIndex() for an invalid Moose variable number. Maybe an AuxVariable?");
    mooseAssert(_arg_numbers[_arg_index[i_var]] == i_var, "Requesting argIndex() for a derivative w.r.t. a variable not coupled to.");
    return _arg_index[i_var];
  }

  /// Coupled variables for function arguments
  std::vector<VariableValue *> _args;

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

  /// Calculate (and allocate memory for) the third derivatives of the free energy.
  bool _third_derivatives;

  /// Material property to store the function value.
  MaterialProperty<Real> * _prop_F;

  /// number of non-linear variables in the problem
  const unsigned int _number_of_nl_variables;

private:
  /// Vector to look up the internal coupled variable index into _arg_*  through the libMesh variable number
  std::vector<unsigned int> _arg_index;
};

#endif //FUNCTIONMATERIALBASE_H
