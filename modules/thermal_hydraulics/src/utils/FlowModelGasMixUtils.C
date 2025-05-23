//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowModelGasMixUtils.h"
#include "VaporMixtureFluidProperties.h"
#include "SinglePhaseFluidProperties.h"

namespace FlowModelGasMixUtils
{

ADReal
computeSecondaryMoleFraction(const ADReal & xi_secondary, const VaporMixtureFluidProperties & fp)
{
  mooseAssert(fp.getNumberOfSecondaryVapors() == 1,
              "This function assumes there is a single secondary fluid.");
  const SinglePhaseFluidProperties & fp_primary = fp.getPrimaryFluidProperties();
  const SinglePhaseFluidProperties & fp_secondary = fp.getSecondaryFluidProperties();

  const ADReal xi_primary = 1 - xi_secondary;

  const ADReal moles_primary = xi_primary / fp_primary.molarMass();
  const ADReal moles_secondary = xi_secondary / fp_secondary.molarMass();

  return moles_secondary / (moles_primary + moles_secondary);
}

}
