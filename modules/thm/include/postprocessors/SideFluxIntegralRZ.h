#pragma once

#include "SideFluxIntegral.h"
#include "RZSymmetry.h"

/**
 * Integrates a diffusive flux over a boundary of a 2D RZ domain.
 */
class SideFluxIntegralRZ : public SideFluxIntegral, public RZSymmetry
{
public:
  static InputParameters validParams();

  SideFluxIntegralRZ(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;
};
