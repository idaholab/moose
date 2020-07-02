//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DirichletBCBase.h"
#include "PeridynamicsMesh.h"

/**
 * Class to apply preset BC of old variable solution based on the number of active bonds.
 * Used to fix nodes with rigid body motion.
 */
class RBMPresetOldValuePD : public DirichletBCBase
{
public:
  static InputParameters validParams();

  RBMPresetOldValuePD(const InputParameters & parameters);

  virtual Real computeQpValue() override;
  virtual bool shouldApply() override;

protected:
  /// Peridynamic mesh
  PeridynamicsMesh & _pdmesh;

  /// Value of the unknown variable this BC is acting on at last time step
  const VariableValue & _u_old;

  /// AuxVariable for number of intact bonds associated with each material point
  MooseVariable * _bond_status_var;
};
