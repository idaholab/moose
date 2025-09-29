//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IPHDGKernel.h"

class MassFluxPenaltyIPHDGAssemblyHelper;

/*
 * Imposes a singular perturbation on the component momentum equations penalizing discontinuities in
 * mass flux. Similar to \p MassFluxPenalty except it does not couple interior degrees of freedom on
 * neighboring elements, which makes this class useful in tandem with hybridized discretizations
 * because it supports static condensation
 */
class MassFluxPenaltyIPHDG : public IPHDGKernel
{
public:
  static InputParameters validParams();

  MassFluxPenaltyIPHDG(const InputParameters & parameters);

protected:
  virtual IPHDGAssemblyHelper & iphdgHelper() override;

private:
  /// The assembly helper providing the required IP-HDG method implementations
  std::unique_ptr<MassFluxPenaltyIPHDGAssemblyHelper> _iphdg_helper;
};
