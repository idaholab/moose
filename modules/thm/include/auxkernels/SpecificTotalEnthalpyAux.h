#pragma once

#include "AuxKernel.h"

/**
 * Nodal auxiliary variable for specific total enthalpy
 *
 * This computes the specific total enthalpy:
 * \f[
 *   H = E + \frac{p}{\rho}
 * \f]
 */
class SpecificTotalEnthalpyAux : public AuxKernel
{
public:
  SpecificTotalEnthalpyAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const VariableValue & _rhoA;
  const VariableValue & _rhoEA;
  const VariableValue & _pressure;
  const VariableValue & _area;
  const VariableValue & _alpha;

public:
  static InputParameters validParams();
};
