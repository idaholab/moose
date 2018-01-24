//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef LEVELSETOLSSONREINITIALIZATION_H
#define LEVELSETOLSSONREINITIALIZATION_H

// MOOSE includes
#include "Kernel.h"

// Forward declarations
class LevelSetOlssonReinitialization;

template <>
InputParameters validParams<LevelSetOlssonReinitialization>();

/**
 * Implements the re-initialization equation proposed by Olsson et. al. (2007).
 */
class LevelSetOlssonReinitialization : public Kernel
{
public:
  LevelSetOlssonReinitialization(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// Gradient of the level set variable at time, \tau = 0.
  const VariableGradient & _grad_levelset_0;

  /// Interface thickness
  const PostprocessorValue & _epsilon;

  ///@{
  /// Helper members to avoid initializing variables in computeQpResidual/Jacobian
  RealVectorValue _f;
  Real _s;
  RealVectorValue _n_hat;
  ///@}
};

#endif // LEVELSETOLSSONREINITIALIZATION_H
