//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"

// Forward Declarations
class VectorPostprocessorReceiver;

template <>
InputParameters validParams<VectorPostprocessorReceiver>();

/**
 * A class for storing data, it allows the user to change the value of the
 * postprocessor by altering the _my_value reference
 */
class VectorPostprocessorReceiver : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   * @param parameters The input parameters
   */
  VectorPostprocessorReceiver(const InputParameters & parameters);

  ///@{
  /**
   * No action taken
   */
  virtual void initialize() override {}
  virtual void execute() override {}
  ///@}

  VectorPostprocessorValue & addVector(std::string name);

private:
  std::map<std::string, VectorPostprocessorValue *> _name_vector_map;
};
