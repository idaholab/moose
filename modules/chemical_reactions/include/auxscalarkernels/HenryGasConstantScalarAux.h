#pragma once

#include "AuxScalarKernel.h"

class HenryGasConstantScalarAux : public AuxScalarKernel
{
public:
  static InputParameters validParams();
  HenryGasConstantScalarAux(const InputParameters & parameters);
  virtual ~HenryGasConstantScalarAux() {}

protected:
  virtual Real computeValue() override;

  /// fluid temperature
  const VariableValue & _temperature;

  /// atomic radius
  const Real _radius;

  /// Enum used to select the type
  const enum class Saltlist { FLIBE, FLINAK, CUSTOM } _salt_list;

  /// Fit coefficients for the model
  Real _alpha;
  Real _beta;
  Real _gamma_0;
  Real _dgamma_dT;
  Real _KH0;
};
