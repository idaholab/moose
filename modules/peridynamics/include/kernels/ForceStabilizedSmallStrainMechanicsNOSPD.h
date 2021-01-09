//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MechanicsBaseNOSPD.h"

/**
 * Kernel class for fictitious force stabilized peridynamic correspondence material model for small
 * strain
 */
class ForceStabilizedSmallStrainMechanicsNOSPD : public MechanicsBaseNOSPD
{
public:
  static InputParameters validParams();

  ForceStabilizedSmallStrainMechanicsNOSPD(const InputParameters & parameters);

protected:
  virtual void computeLocalResidual() override;
  virtual void computeLocalJacobian() override;
  virtual void computeNonlocalJacobian() override;

  void computeLocalOffDiagJacobian(unsigned int jvar_num, unsigned int coupled_component) override;
  void computePDNonlocalOffDiagJacobian(unsigned int jvar_num,
                                        unsigned int coupled_component) override;

  /// Bond based material property for fictitious stabilization force
  const MaterialProperty<Real> & _sf_coeff;

  /// The index of displacement component
  const unsigned int _component;
};
