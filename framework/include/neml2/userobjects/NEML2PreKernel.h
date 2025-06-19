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
#include "MOOSEToNEML2.h"

/// Base class for NEML2 kernels that operate _before_ the constitutive update
class NEML2PreKernel : public NEML2Kernel, public MOOSEToNEML2
{
public:
  static InputParameters validParams();

  NEML2PreKernel(const InputParameters & parameters);

  neml2::Tensor gatheredData() const override { return _output; }
};

#endif
