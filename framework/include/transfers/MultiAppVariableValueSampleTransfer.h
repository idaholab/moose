//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MULTIAPPVARIABLEVALUESAMPLETRANSFER_H
#define MULTIAPPVARIABLEVALUESAMPLETRANSFER_H

// MOOSE includes
#include "MultiAppTransfer.h"

// Forward declarations
class MultiAppVariableValueSampleTransfer;

template <>
InputParameters validParams<MultiAppVariableValueSampleTransfer>();

/**
 * Samples a variable's value in the Master domain at the point where
 * the MultiApp is.  Copies that value into a field in the MultiApp.
 */
class MultiAppVariableValueSampleTransfer : public MultiAppTransfer
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
