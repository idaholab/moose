#pragma once

#include "SideFluxIntegral.h"
#include "RZSymmetry.h"

/**
 * Integrates a diffusive flux over a boundary of a 2D RZ domain.
 */
class ADSideFluxIntegralRZ : public ADSideFluxIntegral, public RZSymmetry
{
public:
  static InputParameters validParams();

  ADSideFluxIntegralRZ(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;
};
