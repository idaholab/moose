//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IPHDGKernel.h"

/**
 * Implements the diffusion equation for a interior penalty hybridized discretization
 */
class DiffusionIPHDGKernel : public IPHDGKernel
{
public:
  static InputParameters validParams();
  DiffusionIPHDGKernel(const InputParameters & params);

protected:
  virtual IPHDGAssemblyHelper & iphdgHelper() override { return *_iphdg_helper; }

private:
  /// The assembly helper providing the required IP-HDG method implementations
  std::unique_ptr<DiffusionIPHDGAssemblyHelper> _iphdg_helper;
};
