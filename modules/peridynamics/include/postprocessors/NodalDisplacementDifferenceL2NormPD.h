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

/**
 * Postprocessor class to compute L2 norm of displacements difference between prediction and
 * analytical solution for peridynamic model
 */
class NodalDisplacementDifferenceL2NormPD : public NodalIntegralPostprocessorBasePD
{
public:
  static InputParameters validParams();

  NodalDisplacementDifferenceL2NormPD(const InputParameters & parameters);

  virtual Real getValue() override;

protected:
  virtual Real computeNodalValue() override;

  /// Displacement variables
  unsigned int _n_disps;
  std::vector<MooseVariable *> _disp_var;

  /// Known analytic displacement functions
  std::vector<const Function *> _funcs;
};
