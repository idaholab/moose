//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalIntegralPostprocessorBasePD.h"

class Function;

/**
 * Postprocessor class to compute L2 norm of a given function for peridynamic discretization
 */
class NodalFunctionsL2NormPD : public NodalIntegralPostprocessorBasePD
{
public:
  static InputParameters validParams();

  NodalFunctionsL2NormPD(const InputParameters & parameters);

  virtual Real getValue() override;

protected:
  virtual Real computeNodalValue() override;

  /// Known functions
  unsigned int _n_funcs;
  std::vector<const Function *> _funcs;
};
