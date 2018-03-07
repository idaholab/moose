//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEBEAMTHERMALEXPANSIONEIGENSTRAINBASE_H
#define COMPUTEBEAMTHERMALEXPANSIONEIGENSTRAINBASE_H

#include "ComputeBeamEigenstrainBase.h"
#include "DerivativeMaterialInterface.h"

class ComputeBeamThermalExpansionEigenstrainBase;

template <>
InputParameters validParams<ComputeBeamThermalExpansionEigenstrainBase>();

/**
 * ComputeBeamThermalExpansionEigenstrainBase is a base class for all models that
 * compute beam eigenstrains due to thermal expansion of a material.
 */
class ComputeBeamThermalExpansionEigenstrainBase
  : public DerivativeMaterialInterface<ComputeBeamEigenstrainBase>
{
public:
  ComputeBeamThermalExpansionEigenstrainBase(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;

  virtual void computeQpEigenstrain() override;
  /*
   * Compute the total thermal strain relative to the stress-free temperature at
   * the current temperature, as well as the current instantaneous thermal
   * expansion coefficient.
   * param thermal_strain    The current total linear thermal strain
   *                         (\delta L / L)
   * param instantaneous_cte The current instantaneous coefficient of thermal
   *                         expansion (derivative of thermal_strain wrt
   *                         temperature
   */
  virtual void computeThermalStrain(Real & thermal_strain, Real & instantaneous_cte) = 0;

  /// Value of temperature at each quadrature point
  const VariableValue & _temperature;

  /// Value of stress free temperature at each quadrature point
  const VariableValue & _stress_free_temperature;

  /// Initial orientation of the beam
  MaterialProperty<RealGradient> & _initial_axis;
};

#endif // COMPUTEBEAMTHERMALEXPANSIONEIGENSTRAINBASE_H
