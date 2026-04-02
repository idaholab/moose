//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ViewFactorBase.h"

/**
 * View factors specified directly in the input file
 */
class SpecifiedViewFactor : public ViewFactorBase
{
public:
  static InputParameters validParams();

  SpecifiedViewFactor(const InputParameters & parameters);

protected:
  virtual void threadJoinViewFactor(const UserObject & /*y*/) override {};
  virtual void finalizeViewFactor() override {};

  virtual void checkViewFactors() const;
};
