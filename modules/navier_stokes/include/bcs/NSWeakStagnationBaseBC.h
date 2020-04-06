//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NSIntegratedBC.h"

// Forward Declarations

// Specialization required of all user-level Moose objects

/**
 * This is the base class for "weakly-imposed" stagnation boundary
 * conditions, that is the relevant boundary integrals are evaluated
 * based on valued implied by fixed stagnation temperature and pressure
 * values and specified flow direction (but not magnitude).
 */
class NSWeakStagnationBaseBC : public NSIntegratedBC
{
public:
  // Constructor
  static InputParameters validParams();

  NSWeakStagnationBaseBC(const InputParameters & parameters);

  // Destructor, better be virtual
  virtual ~NSWeakStagnationBaseBC() {}

protected:
  /**
   * Must be implemented in derived classes.
   */
  // virtual Real computeQpResidual();
  // virtual Real computeQpJacobian();
  // virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Required parameters
  Real _stagnation_pressure;
  Real _stagnation_temperature;

  // Specified flow direction.  Should be the components of
  // a unit vector.  TODO: Test reading RealVectorValue objects
  // directly?
  Real _sx;
  Real _sy;
  Real _sz; // only required in 3D

  //
  // Helper functions...
  //

  // Given |u|, p_0, and T_0, compute static quantities.  Each
  // on depends on the previous, so it makes sense to compute them
  // all even if you don't need them all...
  void staticValues(Real & T_s, Real & p_s, Real & rho_s);

  // Nicer interface if you actually only want one of the static values.
  // Note that they will all still be computed!
  Real rhoStatic();

  // The velocity magnitude, squared
  Real velmag2();

  // The specified flow direction, s, dotted with the outward unit normal
  // normal vector
  Real sdotn();
};
