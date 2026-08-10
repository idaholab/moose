#pragma once

#include "AuxKernel.h"

class KLExpansionUserObject;

class SigmoidTrendAuxBase : public AuxKernel
{
public:
  static InputParameters validParams();
  SigmoidTrendAuxBase(const InputParameters & parameters);

protected:
  Real distanceToSegment(const Point & p) const;
  Real trendValue(Real distance) const;

  const KLExpansionUserObject & _kl_uo;

  const Point _x1;
  const Point _x2;

  const Real _scale_hi;
  const Real _scale_lo;
  const Real _midpoint_of_sigmoid;
  const Real _slope_at_midpoint;
};
