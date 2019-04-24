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

class MechanicsBPD;

template <>
InputParameters validParams<MechanicsBPD>();

/**
 * Kernel class for bond based peridynamic solid mechanics models
 */
class MechanicsBPD : public MechanicsBasePD
{
public:
  MechanicsBPD(const InputParameters & parameters);

protected:
  virtual void computeLocalResidual() override;
  virtual void computeLocalJacobian() override;
  virtual void computeLocalOffDiagJacobian(unsigned int jvar) override;

  ///@{ Bond based material properties
  const MaterialProperty<Real> & _bond_force_ij;
  const MaterialProperty<Real> & _bond_dfdU_ij;
  const MaterialProperty<Real> & _bond_dfdT_ij;
  ///@}

  /// The index of displcement component
  const unsigned int _component;
};
