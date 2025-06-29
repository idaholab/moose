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
 * Compute the critical time step for an explicit integration scheme for heat
 * conduction problems.
 */
class CriticalTimeStepHeatConduction : public ElementPostprocessor
{
public:
  static InputParameters validParams();

  CriticalTimeStepHeatConduction(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void initialSetup() override;

  virtual void finalize() override;
  virtual Real getValue() const override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  /// Density of the material
  const MaterialProperty<Real> & _material_density;

  /// Added density due to mass scaling (zero if no scaling is selected or applied)
  const MaterialProperty<Real> * _density_scaling;

  /// Thermal conductivity of the material
  const MaterialProperty<Real> & _thermal_conductivity;

  /// Specific heat capacity of the material
  const MaterialProperty<Real> & _specific_heat;

  /// User defined factor in the denominator based on the spatial dimensionality.
  /// 1D->2 ; 2D->4 ; 3D->6
  const Real & _dimension_factor;

  /// User defined factor to be multiplied to the critical time step
  const Real & _factor;

  /// Critical time step for explicit solver
  Real _critical_time;
};
