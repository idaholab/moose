//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IPHDGBC.h"

class AdvectionIPHDGAssemblyHelper;

/**
 * Implements an outflow boundary condition for use with a hybridized discretization of
 * the Advection equation
 */
class AdvectionIPHDGOutflowBC : public IPHDGBC
{
public:
  static InputParameters validParams();
  AdvectionIPHDGOutflowBC(const InputParameters & parameters);

protected:
  /**
   * compute the AD residuals
   */
  virtual void compute() override;

  virtual AdvectionIPHDGAssemblyHelper & iphdgHelper() override;

  /// The assembly helper providing the required IP-HDG method implementations
  std::unique_ptr<AdvectionIPHDGAssemblyHelper> _iphdg_helper;

  /// Whether to constrain the Lagrange multiplier to weakly match the interior solution on this
  /// boundary. This should be set to true for pure advection problems and likely false otherwise
  const bool _constrain_lm;
};
