//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MechanicsFiniteStrainBaseNOSPD.h"

class PeridynamicsMesh;

/**
 * Kernel class for Form II of the horizon-stabilized peridynamic correspondence model for
 * finite strain
 */
class HorizonStabilizedFormIIFiniteStrainMechanicsNOSPD : public MechanicsFiniteStrainBaseNOSPD
{
public:
  static InputParameters validParams();

  HorizonStabilizedFormIIFiniteStrainMechanicsNOSPD(const InputParameters & parameters);

protected:
  virtual void computeLocalResidual() override;
  virtual void computeNonlocalResidual() override;

  virtual void computeLocalJacobian() override;
  virtual void computeNonlocalJacobian() override;

  virtual void computeLocalOffDiagJacobian(unsigned int jvar_num,
                                           unsigned int coupled_component) override;
  virtual void computePDNonlocalOffDiagJacobian(unsigned int jvar_num,
                                                unsigned int coupled_component) override;

  /// The index of displacement component
  const unsigned int _component;
};
