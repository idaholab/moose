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

#ifndef MULTIAPPVARIABLEVALUESAMPLETRANSFER_H
#define MULTIAPPVARIABLEVALUESAMPLETRANSFER_H

// MOOSE includes
#include "MultiAppTransfer.h"
#include "MooseVariableDependencyInterface.h"

// Forward declarations
class MultiAppVariableValueSampleTransfer;

template<>
InputParameters validParams<MultiAppVariableValueSampleTransfer>();

/**
 * Samples a variable's value in the Master domain at the point where
 * the MultiApp is.  Copies that value into a field in the MultiApp.
 */
class MultiAppVariableValueSampleTransfer :
  public MultiAppTransfer,
  public MooseVariableDependencyInterface
{
public:
  MultiAppVariableValueSampleTransfer(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual void execute() override;

protected:
  AuxVariableName _to_var_name;
  VariableName _from_var_name;
};

#endif /* MULTIAPPVARIABLEVALUESAMPLETRANSFER_H */
