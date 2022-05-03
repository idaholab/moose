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

class MultiAppVariableValueSamplePostprocessorTransfer : public MultiAppTransfer
{
public:
  static InputParameters validParams();

  MultiAppVariableValueSamplePostprocessorTransfer(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual void execute() override;

protected:
  /// the name of the postprocessor on the sub-applications
  PostprocessorName _postprocessor_name;
  /// the name of the variable on the main-application
  AuxVariableName _var_name;
  /// the component number of the variable for transferring
  unsigned int _comp;
  /// the moose variable
  MooseVariableFieldBase & _var;
  // sub-application ids of all local active element of the main-application
  std::vector<unsigned int> _cached_multiapp_pos_ids;
};
