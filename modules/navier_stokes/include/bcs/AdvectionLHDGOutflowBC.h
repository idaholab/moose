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

class AdvectionLHDGAssemblyHelper;

/**
 * Implements an advective outflow boundary condition using the L-HDG hybrid velocity.
 */
class AdvectionLHDGOutflowBC : public HDGBC
{
public:
  static InputParameters validParams();
  AdvectionLHDGOutflowBC(const InputParameters & parameters);

protected:
  virtual void compute() override;
  virtual HDGAssemblyHelper & hdgHelper() override;

  /// Assembly helper implementing the L-HDG advective boundary flux.
  std::unique_ptr<AdvectionLHDGAssemblyHelper> _lhdg_helper;

  /// Whether this object supplies the facet scalar equation.
  const bool _constrain_lm;
};
