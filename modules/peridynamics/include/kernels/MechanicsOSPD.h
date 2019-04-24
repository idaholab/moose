//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MechanicsBasePD.h"

class MechanicsOSPD;

template <>
InputParameters validParams<MechanicsOSPD>();

/**
 * Kernel class for ordinary state based peridynamic solid mechanics models for small strain
 */
class MechanicsOSPD : public MechanicsBasePD
{
public:
  MechanicsOSPD(const InputParameters & parameters);

protected:
  virtual void computeLocalResidual() override;
  virtual void computeNonlocalResidual() override;

  virtual void computeLocalJacobian() override;
  virtual void computeNonlocalJacobian() override;

  void computeLocalOffDiagJacobian(unsigned int coupled_component) override;
  void computePDNonlocalOffDiagJacobian(unsigned int jvar_num,
                                        unsigned int coupled_component) override;

  ///@{ Bond based material properties
  const MaterialProperty<Real> & _bond_force_ij;
  const MaterialProperty<Real> & _bond_force_i_j;
  const MaterialProperty<Real> & _bond_dfdU_ij;
  const MaterialProperty<Real> & _bond_dfdU_i_j;
  const MaterialProperty<Real> & _bond_dfdT_ij;
  const MaterialProperty<Real> & _bond_dfdT_i_j;
  ///@}

  /// The index of displacement component
  const unsigned int _component;
};
