//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SteadyBase.h"
#include "SIMPLESolveNonlinearAssembly.h"

/**
 * Executioner set up to solve a thermal-hydraulics problem using the
 * SIMPLENonlinearAssembly algorithm. It utilizes segregated linearized systems
 * which are solved using a fixed-point iteration.
 */
class SIMPLENonlinearAssembly : public SteadyBase
{
public:
  static InputParameters validParams();

  SIMPLENonlinearAssembly(const InputParameters & parameters);

  virtual void init() override;

protected:
  /// The SIMPLE solve object that relies on nonlinear assembly routines
  SIMPLESolveNonlinearAssembly _simple_solve;
};
