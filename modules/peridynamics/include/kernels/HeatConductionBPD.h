//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PeridynamicsKernelBase.h"

/**
 * Kernel class for peridynamic heat conduction models
 */
class HeatConductionBPD : public PeridynamicsKernelBase
{
public:
  static InputParameters validParams();

  HeatConductionBPD(const InputParameters & parameters);

protected:
  virtual void computeLocalResidual() override;
  virtual void computeLocalJacobian() override;

  ///@{ Bond based material properties
  const MaterialProperty<Real> & _bond_heat_flow;
  const MaterialProperty<Real> & _bond_dQdT;
  ///@}
};
