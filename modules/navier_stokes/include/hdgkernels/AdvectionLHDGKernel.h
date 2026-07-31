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

class AdvectionLHDGAssemblyHelper;

/**
 * Advection term using a cell velocity in the volume and the velocity trace in face fluxes.
 */
class AdvectionLHDGKernel : public ElementAndTraceScalarHDGKernel
{
public:
  static InputParameters validParams();
  AdvectionLHDGKernel(const InputParameters & params);

private:
  virtual ElementAndTraceScalarHDGAssemblyHelper & hdgHelper() override;

  /// Assembly helper implementing the L-HDG advection weak form.
  std::unique_ptr<AdvectionLHDGAssemblyHelper> _lhdg_helper;
};
