#pragma once

#include "ADNumericalFlux3EqnBase.h"
#include "SinglePhaseFluidProperties.h"

/**
 * Computes internal side flux for the 1-D, 1-phase, variable-area Euler equations using
 * a centered average of the left and right side fluxes
 */
class ADNumericalFlux3EqnCentered : public ADNumericalFlux3EqnBase
{
public:
  ADNumericalFlux3EqnCentered(const InputParameters & parameters);

  virtual void calcFlux(const std::vector<ADReal> & U1,
                        const std::vector<ADReal> & U2,
                        const ADReal & nLR_dot_d,
                        std::vector<ADReal> & FL,
                        std::vector<ADReal> & FR) const override;

  virtual unsigned int getNumberOfRegions() const override { return 1; }

protected:
  std::vector<ADReal> computeFlux(const std::vector<ADReal> & U) const;

  /// fluid properties user object
  const SinglePhaseFluidProperties & _fp;

public:
  static InputParameters validParams();
};
