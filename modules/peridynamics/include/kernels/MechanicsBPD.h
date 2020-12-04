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

/**
 * Kernel class for bond based peridynamic solid mechanics models
 */
class MechanicsBPD : public MechanicsBasePD
{
public:
  static InputParameters validParams();

  MechanicsBPD(const InputParameters & parameters);

protected:
  virtual void computeLocalResidual() override;
  virtual void computeLocalJacobian() override;
  virtual void computeLocalOffDiagJacobian(unsigned int jvar_num, unsigned int jvar) override;

  ///@{ Bond based material properties
  const MaterialProperty<Real> & _bond_local_force;
  const MaterialProperty<Real> & _bond_local_dfdU;
  const MaterialProperty<Real> & _bond_local_dfdT;
  ///@}

  /// The index of displcement component
  const unsigned int _component;
};
