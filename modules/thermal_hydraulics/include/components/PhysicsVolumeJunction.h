//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PhysicsFlowJunction.h"

/**
 * Junction between 1-phase flow channels that has a non-zero volume, using a Physics-based
 * implementation
 */
class PhysicsVolumeJunction : public PhysicsFlowJunction
{
public:
  static InputParameters validParams();

  PhysicsVolumeJunction(const InputParameters & params);

  /// Whether the initial conditions are known
  bool hasInitialConditions() const override;

  /// Return the initial conditions based on the user parameters
  void getInitialConditions(Real & initial_p,
                            Real & initial_T,
                            Real & initial_rho,
                            Real & initial_E,
                            RealVectorValue & inital_vel) const;

protected:
  virtual void setupMesh() override;
  virtual void check() const override;
  virtual void init() override;

  /// Volume of the junction
  const Real _volume;

  /// Spatial position of center of the junction
  const Point & _position;

  /// Form loss coefficient
  const Real & _K;
  /// Reference area
  const Real & _A_ref;
};
