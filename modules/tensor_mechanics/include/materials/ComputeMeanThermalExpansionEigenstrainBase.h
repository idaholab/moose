/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEMEANTHERMALEXPANSIONEIGENSTRAINBASE_H
#define COMPUTEMEANTHERMALEXPANSIONEIGENSTRAINBASE_H

#include "ComputeThermalExpansionEigenstrainBase.h"

class ComputeMeanThermalExpansionEigenstrainBase;

template <>
InputParameters validParams<ComputeMeanThermalExpansionEigenstrainBase>();

/**
 * ComputeMeanThermalExpansionEigenstrainBase is a base class for computing the
 * thermal expansion eigenstrain according to a temperature-dependent mean thermal
 * expansion defined in a derived class.
 */
class ComputeMeanThermalExpansionEigenstrainBase : public ComputeThermalExpansionEigenstrainBase
{
public:
  ComputeMeanThermalExpansionEigenstrainBase(const InputParameters & parameters);

protected:
  virtual void initialSetup() override;

  /*
   * Compute the total thermal strain relative to the stress-free temperature at the
   * current temperature, as well as the current instantaneous thermal expansion coefficient.
   * param thermal_strain    The current total linear thermal strain (\delta L / L)
   * param instantaneous_cte The current instantaneous coefficient of thermal expansion
   *                         (derivative of thermal_strain wrt temperature
   */
  virtual void computeThermalStrain(Real & thermal_strain, Real & instantaneous_cte) override;

  /*
   * Get the reference temperature for the mean thermal expansion relationship
   */
  virtual Real referenceTemperature() = 0;

  /*
   * Compute the total mean thermal expansion relative to the reference temperature.
   * This is the linear thermal strain (\delta L / L)
   * param temperature  temperature at which this is evaluated
   */
  virtual Real meanThermalExpansion(const Real temperature) = 0;

  /*
   * Compute the derivative of the total mean thermal expansion relative to the reference
   * temperature wrt temperature.  This is for the linear thermal strain (\delta L / L)
   * param temperature  temperature at which this is evaluated
   */
  virtual Real meanThermalExpansionDerivative(const Real temperature) = 0;

  /// Mean linear thermal expansion relative to the reference temperature evaluated at stress_free_temperature
  Real _alphabar_stress_free_temperature;

  /// Mean thermal expansion coefficient relative to the reference temperature evaluated at stress_free_temperature
  Real _thexp_stress_free_temperature;
};

#endif // COMPUTEMEANTHERMALEXPANSIONEIGENSTRAINBASE_H
