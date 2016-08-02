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

  virtual void initialSetup() override;

  virtual void execute() override;

protected:
  AuxVariableName _to_var_name;
  VariableName _from_var_name;
};

#endif // MULTIAPPCOPYTRANSFER_H
