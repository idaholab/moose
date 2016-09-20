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

template<>
InputParameters validParams<MultiAppCopyTransfer>();

/**
 * Copy the value to the target domain from the nearest node in the source domain.
 */
class MultiAppCopyTransfer :
  public MultiAppTransfer
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
  void transfer(FEProblem & to_problem, FEProblem & from_problem);

  /**
   * Performs the transfer of values between a node or element.
   */
  void transferDofObject();

  /// The name of the variable to transfer to
  const VariableName & _to_var_name;

  /// Name of variable transfering from
  const VariableName & _from_var_name;

  ///@{
  /// Member data to avoid creating local variables or passing variables via reference to 'transferDofObject' method.
  libMesh::DofObject * _to_object;
  libMesh::DofObject * _from_object;
  unsigned int _to_sys_num;
  unsigned int _to_var_num;
  unsigned int _from_sys_num;
  unsigned int _from_var_num;
  System * _from_sys;
  NumericVector<Real> * _solution;
  MPI_Comm _swapped;
  ///@}
};

#endif // MULTIAPPCOPYTRANSFER_H
