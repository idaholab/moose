//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HDGKernel.h"
#include "IPHDGAssemblyHelper.h"

/**
 * Base kernel for implementing an interior penalty hybridized discretization
 */
class IPHDGKernel : public HDGKernel
{
public:
  static InputParameters validParams();

  IPHDGKernel(const InputParameters & params);

protected:
  /// Returns the IP-HDG helper used by the common HDG assembly implementation.
  virtual IPHDGAssemblyHelper * hdgHelper() override = 0;
};
