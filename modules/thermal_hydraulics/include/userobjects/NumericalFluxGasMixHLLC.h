//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NumericalFluxGasMixBase.h"
#include "NaNInterface.h"

class VaporMixtureFluidProperties;

/**
 * Computes the numerical flux for FlowModelGasMix using
 * the HLLC approximate Riemann solver.
 */
class NumericalFluxGasMixHLLC : public NumericalFluxGasMixBase, public NaNInterface
{
public:
  static InputParameters validParams();

  NumericalFluxGasMixHLLC(const InputParameters & parameters);

  virtual void calcFlux(const std::vector<ADReal> & UL,
                        const std::vector<ADReal> & UR,
                        const RealVectorValue & nLR,
                        const RealVectorValue & t1,
                        const RealVectorValue & t2,
                        std::vector<ADReal> & FL,
                        std::vector<ADReal> & FR) const override;

  virtual unsigned int getNumberOfRegions() const override { return 4; }

protected:
  /**
   * Computes the flow area that is used in the numerical flux
   */
  virtual ADReal computeFlowArea(const std::vector<ADReal> & UL,
                                 const std::vector<ADReal> & UR) const;

  /// fluid properties user object
  const VaporMixtureFluidProperties & _fp;
};
