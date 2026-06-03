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

#include "MooseObjectAction.h"

/**
 * This action allow to introduce MFEM Coordinate coefficient
 * objects, that construct and expose built-in scalar coefficients
 * providers through the '[CoordinateSystem]' block.
 */

class AddMFEMCoordinateCoefficientsAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  AddMFEMCoordinateCoefficientsAction(const InputParameters & params);

  virtual void act() override;
};

#endif
