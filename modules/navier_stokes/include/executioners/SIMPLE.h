//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SteadyBase.h"
#include "SIMPLESolve.h"

/**
 * Executioner set up to solve a thermal-hydraulics problem using the SIMPLE algorithm.
 * It utilizes segregated linear systems which are solved using a fixed-point iteration.
 */
class SIMPLE : public SteadyBase
{
public:
  static InputParameters validParams();

  SIMPLE(const InputParameters & parameters);

  virtual void init() override;

protected:
  SIMPLESolve _simple_solve;
};
