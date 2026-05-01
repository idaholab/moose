#pragma once

#include "GeneralPostprocessor.h"

class MooseMesh;

/**
 * Reconstructs an interface position from neighboring FV cell values by linearly interpolating the
 * threshold crossing between cell centroids on faces aligned with the requested direction.
 */
class SubcellInterfacialPosition : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  SubcellInterfacialPosition(const InputParameters & parameters);

  void initialize() override;
  void execute() override;
  void finalize() override;
  Real getValue() const override;

private:
  bool withinFilterWindow(const Point & point) const;
  void updateCandidate(const Real value);

  const MooseMesh & _mesh;
  const Moose::Functor<Real> & _volume_fraction;
  const unsigned int _direction;
  const bool _search_max;
  const Real _threshold;
  const Real _minimum_alignment;
  const Real _secondary_min;
  const Real _secondary_max;
  const Real _tertiary_min;
  const Real _tertiary_max;
  const Real _value_if_no_interface;

  Real _value;
  bool _found_candidate;
};
