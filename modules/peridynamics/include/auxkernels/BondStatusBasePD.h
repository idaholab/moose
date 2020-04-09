//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernelBasePD.h"

/**
 * Base AuxKernel class for different failure criteria to update the bond status
 * A bond is broken and the bond_status variable has value of 0, if it meets the given failure
 * criterion. If a bond is intact during previous time step and it does not meet the given failure
 * criterion, the bond is taken as intact and the bond_status variable has value of 1.
 */
class BondStatusBasePD : public AuxKernelBasePD
{
public:
  static InputParameters validParams();

  BondStatusBasePD(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /**
   * Function to compute the value of a given failure criterion.
   * Positive value indicates the critical value is exceeded.
   * Otherwise, the critical value is not reached yet
   * @return failure criterion value
   */
  virtual Real computeFailureCriterionValue() = 0;

  /// Bond_status variable
  MooseVariable * _bond_status_var;

  /// Critical AuxVariable
  const VariableValue & _critical_val;
};
