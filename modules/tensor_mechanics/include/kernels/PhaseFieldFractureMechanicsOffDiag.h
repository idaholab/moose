//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PHASEFIELDFRACTUREMECHANICSOFFDIAG_H
#define PHASEFIELDFRACTUREMECHANICSOFFDIAG_H

#include "Kernel.h"
#include "RankTwoTensor.h"
#include "DerivativeMaterialInterface.h"

/**
 * This class computes the off-diagonal Jacobian component of stress divergence residual system
 * Contribution from damage order parameter c
 * Useful if user wants to add the off diagonal Jacobian term
 */

class PhaseFieldFractureMechanicsOffDiag;
class RankTwoTensor;

template <>
InputParameters validParams<PhaseFieldFractureMechanicsOffDiag>();

class PhaseFieldFractureMechanicsOffDiag : public DerivativeMaterialInterface<Kernel>
{
public:
  PhaseFieldFractureMechanicsOffDiag(const InputParameters & parameters);

protected:
  Real computeQpResidual() override { return 0.0; }

  Real computeQpJacobian() override { return 0.0; }

  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  std::string _base_name;
  const unsigned int _component;

  const bool _c_coupled;
  const unsigned int _c_var;
  const MaterialProperty<RankTwoTensor> & _d_stress_dc;
};

#endif // PHASEFIELDFRACTUREMECHANICSOFFDIAG_H
