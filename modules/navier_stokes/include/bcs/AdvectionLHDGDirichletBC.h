//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TwoFieldScalarHDGBC.h"

class AdvectionLHDGAssemblyHelper;

/**
 * Weakly imposes prescribed scalar and velocity data for an L-HDG advection discretization.
 */
class AdvectionLHDGDirichletBC : public TwoFieldScalarHDGBC
{
public:
  static InputParameters validParams();
  AdvectionLHDGDirichletBC(const InputParameters & parameters);

private:
  virtual void compute(TwoFieldScalarHDGAssemblyHelper &) override;
  virtual TwoFieldScalarHDGAssemblyHelper & hdgHelper() override;

  /// Assembly helper implementing the L-HDG advective boundary flux.
  std::unique_ptr<AdvectionLHDGAssemblyHelper> _lhdg_helper;

  /// Prescribed scalar value.
  const Moose::Functor<Real> & _dirichlet_value;
};
