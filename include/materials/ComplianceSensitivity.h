#pragma once

#include "StrainEnergyDensity.h"
#include "MathUtils.h"

class ComplianceSensitivity : public StrainEnergyDensity
{
public:
  static InputParameters validParams();

  ComplianceSensitivity(const InputParameters & parameters);

  virtual void computeQpProperties() override;

protected:
  MaterialProperty<Real> & _sensitivity;
  const VariableValue & _design_density;
  const int _power;
  const Real _E;
  const Real _Emin;
};
