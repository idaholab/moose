//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CrossTermBarrierFunctionBase.h"

// Forward Declarations

/**
 * CrossTermBarrierFunctionMaterial adds free energy contribution on the interfaces
 * between arbitrary pairs of phases in a symmetric way.
 */
class CrossTermBarrierFunctionMaterial : public CrossTermBarrierFunctionBase
{
public:
  static InputParameters validParams();

  CrossTermBarrierFunctionMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();
};
