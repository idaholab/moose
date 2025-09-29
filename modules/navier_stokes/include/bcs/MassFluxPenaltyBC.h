//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IPHDGBC.h"

class MassFluxPenaltyIPHDGAssemblyHelper;

/*
 * Imposes a singular perturbation on the component momentum equations penalizing discontinuities in
 * mass flux. Similar to \p MassFluxPenalty except it does not couple interior degrees of freedom on
 * neighboring elements, which makes this class useful in tandem with hybridized discretizations
 * because it supports static condensation
 */
class MassFluxPenaltyBC : public IPHDGBC
{
public:
  static InputParameters validParams();

  MassFluxPenaltyBC(const InputParameters & parameters);

protected:
  virtual IPHDGAssemblyHelper & iphdgHelper() override;
  virtual void compute() override;

private:
  /// The assembly helper providing the required IP-HDG method implementations
  std::unique_ptr<MassFluxPenaltyIPHDGAssemblyHelper> _iphdg_helper;

  /// Whether this is a Dirichlet boundary for the velocity. If it is, then we will not compute
  /// trace residuals
  const bool _dirichlet_boundary;
};
