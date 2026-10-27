//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementAndTraceScalarHDGKernel.h"

class MassContinuityAssemblyHelper;

/**
 * Implements an advection term for a interior penalty hybridized discretization
 */
class MassContinuityIPHDGKernel : public ElementAndTraceScalarHDGKernel
{
public:
  static InputParameters validParams();
  MassContinuityIPHDGKernel(const InputParameters & params);

private:
  virtual ElementAndTraceScalarHDGAssemblyHelper & hdgHelper() override;

  /// The assembly helper providing the required IP-HDG method implementations
  std::unique_ptr<MassContinuityAssemblyHelper> _iphdg_helper;
};
