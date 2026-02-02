//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMValueSamplerBase.h"

#include "mfem.hpp"

/*
 * MFEM Postprocessor which samples values at a set of points evenly distributed along a line.
 */
class MFEMLineValueSampler : public MFEMValueSamplerBase
{
public:
  static InputParameters validParams();

  MFEMLineValueSampler(const InputParameters & parameters);
};

#endif // MOOSE_MFEM_ENABLED
