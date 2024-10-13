//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiAppTransfer.h"

class MooseVariableFieldBase;

/**
 * Intermediary class that allows variable names as inputs
 */
class MultiAppFieldTransfer : public MultiAppTransfer
{
public:
  static InputParameters validParams();

  MultiAppFieldTransfer(const InputParameters & parameters);

  virtual void initialSetup();

protected:
  /// Virtual function defining variables to be transferred
  virtual std::vector<VariableName> getFromVarNames() const = 0;
  /// Virtual function defining variables to transfer to
  virtual std::vector<AuxVariableName> getToVarNames() const = 0;

  /// Returns the Problem's equation system, displaced or not
  /// Be careful! If you transfer TO a displaced system you will likely need a synchronization
  /// So most transfers reach the non-displaced system directly
  libMesh::EquationSystems & getEquationSystem(FEProblemBase & problem, bool use_displaced) const;
};
