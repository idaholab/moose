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
 * Postprocessor to commpute the number of bonds whose stataus has changed in the most update.
 Used to check whether the Picard iterations have converged for the current time step.
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
  virtual void finalize() override;

protected:
  /// Bond status aux variable
  MooseVariable * _bond_status_var;

  /// Reference to peridynamic mesh object
  PeridynamicsMesh & _pdmesh;

  /// Mesh dimension
  const unsigned int _dim;

  /// total number of times bond_status updated
  unsigned int _num_bond_status_updated;
};
