/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
