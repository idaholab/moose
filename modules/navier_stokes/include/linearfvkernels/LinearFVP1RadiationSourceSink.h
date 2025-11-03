//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVElementalKernel.h"

/**
 * Kernel that adds contributions from a external source term discretized using the finite volume
 * method to a linear system.
 */
class LinearFVP1RadiationSourceSink : public LinearFVElementalKernel
{
public:
  static InputParameters validParams();

  /**
   * Class constructor.
   * @param params The InputParameters for the kernel.
   */
  LinearFVP1RadiationSourceSink(const InputParameters & params);

  virtual Real computeMatrixContribution() override;

  virtual Real computeRightHandSideContribution() override;

protected:

  /// The functor for the radiation temperature
  const Moose::Functor<Real> & _temperature_radiation;

  /// The functor for the absorption coefficient
  const Moose::Functor<Real> & _sigma_a;
};
