#pragma once

#include "InitialCondition.h"

// Forward Declarations
class SpecificTotalEnthalpyIC;

template <>
InputParameters validParams<SpecificTotalEnthalpyIC>();

/**
 * Initial condition for specific total enthalpy
 *
 * This computes the specific total enthalpy:
 * \f[
 *   H = E + \frac{p}{\rho}
 * \f]
 */
class SpecificTotalEnthalpyIC : public InitialCondition
{
public:
  SpecificTotalEnthalpyIC(const InputParameters & parameters);

protected:
  virtual Real value(const Point & /*p*/);

  const VariableValue & _rhoA;
  const VariableValue & _rhoEA;
  const VariableValue & _pressure;
  const VariableValue & _area;
  const VariableValue & _alpha;
};
