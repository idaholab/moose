//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalBC.h"
#include "PeridynamicsMesh.h"

class IntactBondsDirichletBCPD;

template <>
InputParameters validParams<IntactBondsDirichletBCPD>();

/**
 * Defines Dirichlet boundary condition based on number of intact bonds associated with each
 * material point
 */
class IntactBondsDirichletBCPD : public NodalBC
{
public:
  IntactBondsDirichletBCPD(const InputParameters & parameters);

  virtual Real computeQpResidual() override;
  virtual bool shouldApply() override;

protected:
  /// Peridynamic mesh
  PeridynamicsMesh & _pdmesh;

  /// Value of the unknown variable this BC is acting on at last time step
  const VariableValue & _u_old;

  /// value of the AuxVariable for number of intact bonds associated with each material point
  const VariableValue & _intact_bonds_val;
};
