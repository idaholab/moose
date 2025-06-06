//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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

  virtual void calcFlux(const std::vector<ADReal> & UL,
                        const std::vector<ADReal> & UR,
                        const RealVectorValue & nLR,
                        const RealVectorValue & t1,
                        const RealVectorValue & t2,
                        std::vector<ADReal> & FL,
                        std::vector<ADReal> & FR) const override;

  virtual unsigned int getNumberOfRegions() const override { return 4; }

protected:
  /// Type for how to compute left and right wave speeds
  enum class WaveSpeedFormulation
  {
    EINFELDT,
    DAVIS
  };

  /**
   * Computes the flow area that is used in the numerical flux
   */
  virtual ADReal computeFlowArea(const std::vector<ADReal> & UL,
                                 const std::vector<ADReal> & UR) const;

  /// fluid properties user object
  const SinglePhaseFluidProperties & _fp;

  /// How to compute left and right wave speeds
  const WaveSpeedFormulation _wave_speed_formulation;

public:
  static InputParameters validParams();
};
