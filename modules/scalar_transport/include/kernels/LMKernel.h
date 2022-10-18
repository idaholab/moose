//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelValue.h"

class LMKernel : public ADKernelValue
{
public:
  LMKernel(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  void computeResidual() override;
  void computeResidualsForJacobian() override;

  MooseVariable & _lm_var;
  const ADVariableValue & _lm;
  const VariableTestValue & _lm_test;
  const Real _lm_sign;

private:
  const std::vector<dof_id_type> & dofIndices() const override { return _all_dof_indices; }

  /// The union of the primary var dof indices as well as the LM dof indices
  std::vector<dof_id_type> _all_dof_indices;
};
