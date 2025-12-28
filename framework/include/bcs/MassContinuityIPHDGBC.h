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
#include "MassContinuityAssemblyHelper.h"

/**
 * Applies a Dirichlet value for velocity to mass conservation terms on boundary faces
 */
class MassContinuityIPHDGBC : public IPHDGBC
{
public:
  static InputParameters validParams();
  MassContinuityIPHDGBC(const InputParameters & parameters);

protected:
  virtual void compute() override;
  virtual IPHDGAssemblyHelper & iphdgHelper() override { return *_iphdg_helper; }

  /// The assembly helper providing the required IP-HDG method implementations
  std::unique_ptr<MassContinuityAssemblyHelper> _iphdg_helper;
};
