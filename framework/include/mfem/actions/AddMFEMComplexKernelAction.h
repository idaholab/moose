//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#pragma once

#include "MooseObjectAction.h"
#include "MFEMProblem.h"
/**
 * This class allows us to have a section of the input file like the
 * following to add MFEM coefficients to the problem.
 *
 * [ComplexKernels]
 * []
 */
class AddMFEMComplexKernelAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  AddMFEMComplexKernelAction(const InputParameters & parameters);

  virtual void act() override;
};

#endif
