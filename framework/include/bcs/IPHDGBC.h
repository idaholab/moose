//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HDGBC.h"
#include "IPHDGAssemblyHelper.h"

/**
 * Implements a prescribed flux for an IP-HDG discretization
 */
class IPHDGBC : public HDGBC
{
public:
  static InputParameters validParams();

  IPHDGBC(const InputParameters & parameters);

protected:
  /**
   * compute the AD residuals
   */
  virtual void compute() override = 0;

  /// Returns the IP-HDG helper used by the common HDG boundary assembly implementation.
  virtual IPHDGAssemblyHelper & hdgHelper() override = 0;
};
