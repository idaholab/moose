//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GBDependentTensorBase.h"

/**
 * GB dependent anisotropic tensor Ref. Forest, MSMSE, 2015
 */
class GBDependentAnisotropicTensor : public GBDependentTensorBase
{
public:
  static InputParameters validParams();

  GBDependentAnisotropicTensor(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();
};
