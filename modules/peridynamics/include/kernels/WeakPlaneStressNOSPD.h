//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef WEAKPLANESTRESSNOSPD_H
#define WEAKPLANESTRESSNOSPD_H

#include "MechanicsBaseNOSPD.h"

class WeakPlaneStressNOSPD;

template <>
InputParameters validParams<WeakPlaneStressNOSPD>();

/**
 * Kernel class for weak plane stress formulation based on bond-associated correspondence material
 * model
 */
class WeakPlaneStressNOSPD : public MechanicsBaseNOSPD
{
public:
  WeakPlaneStressNOSPD(const InputParameters & parameters);

  virtual void computeLocalResidual() override;
  virtual void computeLocalJacobian() override;
  virtual void computeLocalOffDiagJacobian(unsigned int coupled_component) override;
  virtual void computePDNonlocalOffDiagJacobian(unsigned int jvar_num,
                                                unsigned int coupled_component) override;
};
#endif // WEAKPLANESTRESSNOSPD_H
