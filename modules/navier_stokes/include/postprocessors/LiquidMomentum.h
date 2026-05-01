#pragma once

#include "GeneralPostprocessor.h"

class MooseMesh;

/**
 * Computes one component of the integrated liquid momentum.
 */
class LiquidMomentum : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  LiquidMomentum(const InputParameters & parameters);

  void initialize() override;
  void execute() override;
  void finalize() override;
  Real getValue() const override;

private:
  const MooseMesh & _mesh;
  const Moose::Functor<Real> & _volume_fraction;
  const Moose::Functor<Real> & _liquid_density;
  const Moose::Functor<Real> & _velocity_component;

  Real _momentum;
};
