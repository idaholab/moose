//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PresetNodalBC.h"
#include "PeridynamicsMesh.h"

class IntactBondsPresetBCPD;

template <>
InputParameters validParams<IntactBondsPresetBCPD>();

/**
 * Class to selectively apply a preset Dirichlet BC based on the number of intact
 * bonds associated with each material point. Used to stabilize nodes without
 * a sufficient number of connections to other material points.
 */
class IntactBondsPresetBCPD : public PresetNodalBC
{
public:
  IntactBondsPresetBCPD(const InputParameters & parameters);

  virtual Real computeQpValue() override;
  virtual bool shouldApply() override;

protected:
  /// Peridynamic mesh
  PeridynamicsMesh & _pdmesh;

  /// Value of the unknown variable this BC is acting on at last time step
  const VariableValue & _u_old;

  /// Bond_status variable
  const MooseVariable & _bond_status_var;

  /// Maximum number of intact bonds connected a node for this BC to be active
  const unsigned int _max_intact_bonds;
};
