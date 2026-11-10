//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SpringLawBase.h"

/**
 * Viscous spring law (a dashpot): the force along each local axis is a constant times the
 * rate of the deformation along that axis.
 */
class ViscousSpring : public SpringLawBase
{
public:
  static InputParameters validParams();

  ViscousSpring(const InputParameters & parameters);

protected:
  virtual void computeLaw(const RealVectorValue & deformation,
                          RealVectorValue & force,
                          RankTwoTensor & stiffness) override;

  /// Damping coefficient along each local axis
  const RealVectorValue _damping;

  /// Rate of the deformation (or rotation) and the derivative of the rate with respect to the
  /// solution, from ComputeSpringDeformation with compute_rates = true
  const MaterialProperty<RealVectorValue> & _rate;
  const MaterialProperty<Real> & _rate_du_dot_du;
};
