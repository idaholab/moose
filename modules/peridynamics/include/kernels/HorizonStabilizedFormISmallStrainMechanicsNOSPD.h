//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MechanicsBaseNOSPD.h"

class PeridynamicsMesh;

/**
 * Kernel class for Form I of the horizon-stabilized peridynamic correspondence model for small
 * strain
 */
class HorizonStabilizedFormISmallStrainMechanicsNOSPD : public MechanicsBaseNOSPD
{
public:
  static InputParameters validParams();

  HorizonStabilizedFormISmallStrainMechanicsNOSPD(const InputParameters & parameters);

protected:
  virtual void computeLocalResidual() override;

  virtual void computeLocalJacobian() override;
  virtual void computeNonlocalJacobian() override;

  virtual void computeLocalOffDiagJacobian(unsigned int jvar_num,
                                           unsigned int coupled_component) override;
  virtual void computePDNonlocalOffDiagJacobian(unsigned int jvar_num,
                                                unsigned int coupled_component) override;

  /// The index of displacement component
  const unsigned int _component;
};
