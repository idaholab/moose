//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxScalarKernel.h"
#include "SurrogateModel.h"

/**
 * Sets a value of a scalar variable based on a surrogate model
 */
class SurrogateModelScalarAux : public AuxScalarKernel, SurrogateModelInterface
{
public:
  static InputParameters validParams();

  SurrogateModelScalarAux(const InputParameters & parameters);
  void initialSetup() override;

protected:
  virtual Real computeValue() override;

  /// Pointers to surrogate model
  const SurrogateModel & _model;

  /// number of parameters
  unsigned int _n_params;

  ///@{ the real parameters that _model is evaluated at
  std::vector<const Real *> _scalar_var_params;
  std::vector<unsigned int> _scalar_var_indices;
  unsigned int _n_scalar;
  ///@}

  ///@{ the pp parameters that _model is evaluated at
  std::vector<const PostprocessorValue *> _pp_params;
  std::vector<unsigned int> _pp_indices;
  unsigned int _n_pp;
  ///@}

  ///@{ the real parameters that _model is evaluated at
  std::vector<Real> _real_params;
  std::vector<unsigned int> _real_indices;
  unsigned int _n_real;
  ///@}
};
