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
#include "SurrogateModel.h"

// forward declarations
template <typename ComputeValueType>
class SurrogateModelAuxKernelTempl;
class FEProblemBase;

typedef SurrogateModelAuxKernelTempl<Real> SurrogateModelAuxKernel;
typedef SurrogateModelAuxKernelTempl<RealEigenVector> SurrogateModelArrayAuxKernel;

/**
 * Sets a value of auxiliary variables based on a surrogate model
 */
template <typename ComputeValueType>
class SurrogateModelAuxKernelTempl : public AuxKernelTempl<ComputeValueType>,
                                     public SurrogateModelInterface
{
public:
  static InputParameters validParams();

  SurrogateModelAuxKernelTempl(const InputParameters & parameters);

protected:
  virtual ComputeValueType computeValue() override;

  /// Pointers to surrogate model
  const SurrogateModel & _model;

  /// number of parameters
  const unsigned int _n_params;

  /// The pp parameters that _model is evaluated at
  std::map<unsigned int, const PostprocessorValue *> _pp_params;
  /// The function parameters that _model is evaluated at
  std::map<unsigned int, const Function *> _function_params;
  /// The standard variable parameters that _model is evaluated at
  std::map<unsigned int, const VariableValue *> _var_params;
  /// The array variable parameters that _model is evaluated at
  std::map<unsigned int, const ArrayVariableValue *> _array_var_params;
  /// Constant parameters that _model is evaluated at
  std::map<unsigned int, Real> _constant_params;
};
