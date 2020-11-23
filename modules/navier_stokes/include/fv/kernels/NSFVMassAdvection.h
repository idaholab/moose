//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NSFVAdvectionKernel.h"

#ifdef MOOSE_GLOBAL_AD_INDEXING

/**
 * A flux kernel transporting mass across cell faces
 */
class NSFVMassAdvection : public NSFVAdvectionKernel
{
public:
  static InputParameters validParams();
  NSFVMassAdvection(const InputParameters & params);
};

#endif
