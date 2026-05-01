#pragma once

#include "GeneralPostprocessor.h"

class MooseMesh;

/**
 * Computes one coordinate of the liquid center of mass from a volume fraction and liquid density.
 */
class LiquidCenterOfMass : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  LiquidCenterOfMass(const InputParameters & parameters);

  void initialize() override;
  void execute() override;
  void finalize() override;
  Real getValue() const override;

private:
  const MooseMesh & _mesh;
  const Moose::Functor<Real> & _volume_fraction;
  const Moose::Functor<Real> & _liquid_density;
  const unsigned int _direction;
  const Real _fallback_value;

  Real _weighted_coordinate_sum;
  Real _mass_sum;
  Real _value;
};
