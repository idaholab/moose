//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MULTIAPPVARIABLEVALUESAMPLEPOSTPROCESSORTRANSFER_H
#define MULTIAPPVARIABLEVALUESAMPLEPOSTPROCESSORTRANSFER_H

#include "MultiAppTransfer.h"

// Forward declarations
class MultiAppVariableValueSamplePostprocessorTransfer;

template <>
InputParameters validParams<MultiAppVariableValueSamplePostprocessorTransfer>();

/**
 * Samples a variable's value in the Master domain at the point where
 * the MultiApp is.  Copies that value into a postprocessor in the
 * MultiApp.
 */
class MultiAppVariableValueSamplePostprocessorTransfer : public MultiAppTransfer
{
public:
  MultiAppVariableValueSamplePostprocessorTransfer(const InputParameters & parameters);

  virtual void execute() override;

protected:
  AuxVariableName _postprocessor_name;
  PostprocessorName _from_var_name;
};

#endif // MULTIAPPVARIABLEVALUESAMPLEPOSTPROCESSORTRANSFER_H
