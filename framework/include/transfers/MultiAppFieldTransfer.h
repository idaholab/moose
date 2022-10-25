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
namespace libMesh
{
class DofObject;
}

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
  /**
   * Performs the transfer of a variable between two problems if they have the same mesh.
   */
  void transfer(FEProblemBase & to_problem, FEProblemBase & from_problem);

  /**
   * Performs the transfer of values between a node or element.
   */
  void transferDofObject(libMesh::DofObject * to_object,
                         libMesh::DofObject * from_object,
                         MooseVariableFieldBase & to_var,
                         MooseVariableFieldBase & from_var,
                         NumericVector<Number> & to_solution,
                         NumericVector<Number> & from_solution);

  /// Virtual function defining variables to be transferred
  virtual std::vector<VariableName> getFromVarNames() const = 0;
  /// Virtual function defining variables to transfer to
  virtual std::vector<AuxVariableName> getToVarNames() const = 0;

  /// Returns the Problem's equation system, displaced or not
  /// Be careful! If you transfer TO a displaced system you will likely need a synchronization
  /// So most transfers reach the non-displaced system directly
  EquationSystems & getEquationSystem(const FEProblemBase & problem, bool use_displaced) const;

  /// Whether block restriction is active
  const bool _has_block_restrictions;
  /// Subdomain IDs of the blocks to transfer from
  std::set<SubdomainID> _from_blocks;
  /// Subdomain IDs of the blocks to transfer to
  std::set<SubdomainID> _to_blocks;
};
