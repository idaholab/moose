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

#ifndef MULTIAPPSEEDTRANSFER_H
#define MULTIAPPSEEDTRANSFER_H

#include "MultiAppTransfer.h"

// Forward declarations
class MultiAppSeedTransfer;
class MooseVariable;

template<>
InputParameters validParams<MultiAppSeedTransfer>();

/**
 * Copy the value to the target domain from the nearest node in the source domain.
 */
class MultiAppSeedTransfer :
  public MultiAppTransfer
{
public:
  MultiAppSeedTransfer(const InputParameters & parameters);

  /**
   * Performs the transfer of a variable (Nonlinear or Auxiliary) to/from the Multiapp.
   */
  virtual void execute() override;

protected:

  /**
   * Performs the transfer of a variable between two problems.
   */
  void transfer(FEProblemBase & to_problem, FEProblemBase & from_problem);
};

#endif // MULTIAPPSEEDTRANSFER_H
