//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementPostprocessor.h"
#include "PeridynamicsMesh.h"

// Forward Declarations

/**
 * Postprocessor to check whether the bond status update is converged for a given timestep
 */
class BondStatusConvergedPostprocessorPD : public ElementPostprocessor
{
public:
  static InputParameters validParams();

  BondStatusConvergedPostprocessorPD(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & uo) override;
  virtual Real getValue() override;

protected:
  /// Bond status aux variable
  MooseVariable * _bond_status_var;

  /// Reference to peridynamic mesh object
  PeridynamicsMesh & _pdmesh;

  /// Mesh dimension
  const unsigned int _dim;

  /// total number of times bond_status updated
  unsigned int _bond_status_updated_times;
};
