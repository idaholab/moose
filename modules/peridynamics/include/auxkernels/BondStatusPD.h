//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef BONDSTATUSPD_H
#define BONDSTATUSPD_H

#include "AuxKernelBasePD.h"
#include "RankTwoTensor.h"

class BondStatusPD;

template <>
InputParameters validParams<BondStatusPD>();

/**
 * Aux Kernel class to update the bond status during fracture modeling
 * A bond is broken and the bond_status variable has value of 0, if it meets the given failure
 * criterion. If a bond is intact during previous time step and it does not meet the given failure
 * criterion, the bond is taken as intact and the bond_status variable has value of 1.
 */
class BondStatusPD : public AuxKernelBasePD
{
public:
  BondStatusPD(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Enum class used to control which failure criterion to use
  enum class FailureCriterion
  {
    CriticalStretch,
    MaximumTensileStress
  };

  /// Enum used to control which failure criterion to use
  const FailureCriterion _failure_criterion;

  /// Bond_status variable
  const MooseVariableFEBase & _bond_status_var;

  /// Critical AuxVariable
  const VariableValue & _critical_val;

  /// Material property containing the mechanical stretch
  const MaterialProperty<Real> & _mechanical_stretch;

  /// Material property containing the stress
  const MaterialProperty<RankTwoTensor> * _stress;
};

#endif // BONDSTATUSPD_H
