//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

/**
 * Sets up all the necessary kernels for the anisotropic grain growth epsilon and gamma models
 */
class SphericalGaussianKernelAction : public Action
{
public:
  static InputParameters validParams();

  SphericalGaussianKernelAction(const InputParameters & params);

  virtual void act();

protected:
  /// Type of models
  enum class ModelType
  {
    EPSILON,
    GAMMA
  };

  /// Choose between the epsilon and gamma models
  const ModelType _model_type;

  /// Number of coupled order parameter variables
  const unsigned int _op_num;

  /// Base name for the coupled order parameter variables
  const std::string _var_name_base;
};
