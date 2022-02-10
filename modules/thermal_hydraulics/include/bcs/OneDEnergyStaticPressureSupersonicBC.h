//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "OneDEnergyFreeBC.h"

class SinglePhaseFluidProperties;

/**
 *
 */
class OneDEnergyStaticPressureSupersonicBC : public OneDEnergyFreeBC
{
public:
  OneDEnergyStaticPressureSupersonicBC(const InputParameters & parameters);

protected:
  virtual bool shouldApply();

  const bool & _reversible;
  const VariableValue & _vel_old;
  const VariableValue & _v_old;
  const VariableValue & _e_old;
  const MaterialProperty<Real> & _c;

  const SinglePhaseFluidProperties & _fp;

public:
  static InputParameters validParams();
};
