#pragma once

#include "FVElementalKernel.h"

/**
 * Implements a heat transfer term with an ambient medium, proportional to the
 * difference between the fluid and ambient temperature.
 */
class NSFVEnergyAmbientConvection : public FVElementalKernel
{
public:
  static InputParameters validParams();

  NSFVEnergyAmbientConvection(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  /// the convective heat transfer coefficient
  const Moose::Functor<ADReal> & _alpha;
  /// the ambient temperature of the medium with which the fluid exchanges heat
  const Moose::Functor<ADReal> & _temp_ambient;
};
