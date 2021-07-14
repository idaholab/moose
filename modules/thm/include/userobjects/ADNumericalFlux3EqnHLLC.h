#pragma once

#include "ADNumericalFlux3EqnBase.h"
#include "SinglePhaseFluidProperties.h"
#include "NaNInterface.h"

/**
 * Computes internal side flux for the 1-D, 1-phase, variable-area Euler equations using
 * the HLLC approximate Riemann solver.
 *
 * The approach in the following reference for the 3-D Euler equations was
 * extended to the 1-D, variable-area Euler equations:
 *
 * Batten, P., Leschziner, M. A., & Goldberg, U. C. (1997).
 * Average-state Jacobians and implicit methods for compressible viscous and turbulent flows.
 * Journal of computational physics, 137(1), 38-78.
 */
class ADNumericalFlux3EqnHLLC : public ADNumericalFlux3EqnBase, public NaNInterface
{
public:
  ADNumericalFlux3EqnHLLC(const InputParameters & parameters);

  virtual void calcFlux(const std::vector<ADReal> & U1,
                        const std::vector<ADReal> & U2,
                        const ADReal & nLR_dot_d,
                        std::vector<ADReal> & FL,
                        std::vector<ADReal> & FR) const override;

  virtual unsigned int getNumberOfRegions() const override { return 4; }

protected:
  /// fluid properties user object
  const SinglePhaseFluidProperties & _fp;

public:
  static InputParameters validParams();
};
