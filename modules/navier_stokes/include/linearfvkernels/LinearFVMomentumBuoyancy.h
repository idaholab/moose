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
 * Kernel that adds the component of the Boussinesq term in the momentum
 * equations to the right hand side.
 */
class LinearFVMomentumBuoyancy : public LinearFVElementalKernel
{
public:
  static InputParameters validParams();

  /**
   * Class constructor.
   * @param params The InputParameters for the kernel.
   */
  LinearFVMomentumBuoyancy(const InputParameters & params);

  virtual Real computeMatrixContribution() override;

  virtual Real computeRightHandSideContribution() override;

protected:
  /// Index x|y|z of the momentum equation component
  const unsigned int _index;
  /// the density
  const Moose::Functor<Real> & _rho;
  /// The gravity vector
  const RealVectorValue _gravity;
  /// The reference point vector
  const RealVectorValue _ref_point;
};
