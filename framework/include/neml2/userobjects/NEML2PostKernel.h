//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef NEML2_ENABLED

// MOOSE includes
#include "NEML2Kernel.h"
#include "NEML2ModelExecutor.h"

/// Base class for NEML2 kernels that operate _after_ the constitutive update
class NEML2PostKernel : public NEML2Kernel
{
public:
  static InputParameters validParams();

  NEML2PostKernel(const InputParameters & parameters);

protected:
  /// the constitutive model
  const NEML2ModelExecutor & _constitutive;
};

#endif
