//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"
#include "ControlData.h"

class THMProblem;

/**
 * Reads a control value data and prints it out
 */
class RealControlDataValuePostprocessor : public GeneralPostprocessor
{
public:
  RealControlDataValuePostprocessor(const InputParameters & parameters);

  virtual void initialize();
  virtual Real getValue();
  virtual void execute();

protected:
  THMProblem * _thm_problem;
  /// The name of the control data value
  const std::string & _control_data_name;
  /// The value of the control data
  const ControlData<Real> * _control_data_value;

public:
  static InputParameters validParams();
};
