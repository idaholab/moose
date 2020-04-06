//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxScalarKernel.h"

class SolutionUserObject;

/**
 * AuxScalarKernel for reading a solution from file.
 * Creates a function that extracts values from a solution read from a file,
 * via a SolutionUserObject. It is possible to scale and add a constant to the
 * solution read.
 */
class SolutionScalarAux : public AuxScalarKernel
{
public:
  static InputParameters validParams();

  SolutionScalarAux(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  virtual Real computeValue() override;

  /// Reference to the SolutionUserObject storing the solution
  const SolutionUserObject & _solution_object;

  /// The variable name of interest
  std::string _var_name;

  /// Multiplier for the solution, the a of ax+b
  const Real _scale_factor;

  /// Additional factor added to the solution, the b of ax+b
  const Real _add_factor;
};
