/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEMEANTHERMALEXPANSIONFUNCTIONEIGENSTRAIN_H
#define COMPUTEMEANTHERMALEXPANSIONFUNCTIONEIGENSTRAIN_H

#include "ComputeMeanThermalExpansionEigenstrainBase.h"

class ComputeMeanThermalExpansionFunctionEigenstrain;

template <>
InputParameters validParams<ComputeMeanThermalExpansionFunctionEigenstrain>();

/**
 * ComputeMeanThermalExpansionFunctionEigenstrain computes an eigenstrain for thermal
 * expansion according to a mean thermal expansion function.
 */
class ComputeMeanThermalExpansionFunctionEigenstrain
    : public ComputeMeanThermalExpansionEigenstrainBase
{
public:
  ComputeMeanThermalExpansionFunctionEigenstrain(const InputParameters & parameters);

protected:
  /*
   * Get the reference temperature for the mean thermal expansion relationship
   */
  virtual Real referenceTemperature() override;

  /*
   * Compute the total mean thermal expansion relative to the reference temperature.
   * This is the linear thermal strain (\delta L / L)
   * param temperature  temperature at which this is evaluated
   */
  virtual Real meanThermalExpansion(const Real temperature) override;

  /*
   * Compute the derivative of the total mean thermal expansion relative to the reference
   * temperature wrt temperature.  This is for the linear thermal strain (\delta L / L)
   * param temperature  temperature at which this is evaluated
   */
  virtual Real meanThermalExpansionDerivative(const Real temperature) override;

  Function & _thermal_expansion_function;

  const Real & _thexp_func_ref_temp;
};

#endif // COMPUTEMEANTHERMALEXPANSIONFUNCTIONEIGENSTRAIN_H
