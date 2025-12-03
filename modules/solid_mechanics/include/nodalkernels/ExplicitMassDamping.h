//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalKernel.h"

// Forward Declarations

/**
 * Calculates eta * mass * velocity for use in ExplicitMixedOrder time integration
 */
class ExplicitMassDamping : public NodalKernel
{
public:
  static InputParameters validParams();

  ExplicitMassDamping(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  /// Damping coefficient
  const Real _eta;

  /// The lumped mass matrix
  const NumericVector<Number> & _mass_matrix_lumped;

  /// The old velocity
  const Real & _u_dot_old;

private:
  /**
   * Initialize the lumped mass matrix.
   * @return lumped mass matrix vector
   */
  const NumericVector<Number> & initLumpedMass();
};
