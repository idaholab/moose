//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MULTIAPPCOPYTRANSFER_H
#define MULTIAPPCOPYTRANSFER_H

#include "MultiAppTransfer.h"

// Forward declarations
class MultiAppCopyTransfer;
class MooseVariable;

template <>
InputParameters validParams<MultiAppCopyTransfer>();

/**
 * Copy the value to the target domain from the nearest node in the source domain.
 */
class MultiAppCopyTransfer : public MultiAppTransfer
{
public:
  MultiAppCopyTransfer(const InputParameters & parameters);

  /**
   * Performs basic error checking that the variable exists on MultiApp.
   */
  virtual void initialSetup() override;

  /**
   * Performs the transfer of a variable (Nonlinear or Auxiliary) to/from the Multiapp.
   */
  virtual void execute() override;

protected:
  /**
   * Performs the transfer of a variable between two problems.
   */
  void transfer(FEProblemBase & to_problem, FEProblemBase & from_problem);

  /**
   * Performs the transfer of values between a node or element.
   */
  void transferDofObject(libMesh::DofObject * to_object,
                         libMesh::DofObject * from_object,
                         MooseVariable & to_var,
                         MooseVariable & from_var);

  /// The name of the variable to transfer to
  const VariableName & _to_var_name;

  /// Name of variable transfering from
  const VariableName & _from_var_name;
};

#endif // MULTIAPPCOPYTRANSFER_H
