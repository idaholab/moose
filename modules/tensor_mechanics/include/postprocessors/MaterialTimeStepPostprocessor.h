//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementPostprocessor.h"

/**
 * This postporocessor calculates an estimated timestep size that limits
 * an auxiliary variable to below a given threshold.
 */
class MaterialTimeStepPostprocessor : public ElementPostprocessor
{
public:
  static InputParameters validParams();

  MaterialTimeStepPostprocessor(const InputParameters & parameters);
  virtual void initialize();
  virtual void execute();
  virtual Real getValue();
  virtual void finalize();
  virtual void threadJoin(const UserObject & y);

protected:
  /// Flag to find the time step limit from material properties
  const bool _use_material_timestep_limit;

  /// Pointer to the material property containing the time step limit
  const MaterialProperty<Real> * const _matl_time_step;

  /// Current time step limit from the material properties
  Real _matl_value;

  /// Flag to limit the time step based on the number of elements changed
  const bool _use_elements_changed;

  ///@{ Material property used to determine if elements have changed
  const MaterialProperty<Real> * const _changed_property;
  const MaterialProperty<Real> * const _changed_property_old;
  ///@}

  /// Target number of changed elements used to determine if we need to change the time step
  const int _elements_changed;

  /// Current number of elements changed
  int _count;

  /// Tolerance to determine if elements have changed
  const Real _elements_changed_threshold;

  /// Maximum allowed value
  const Real _max;

  /// Current quadrature point
  unsigned int _qp;
};
