//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#pragma once

#include "GeneralVectorPostprocessor.h"

/**
 * A UserObject that tests the TorchScriptModule.
 */
class TorchScriptModuleTest : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  TorchScriptModuleTest(const InputParameters & params);

  virtual void initialize(){};
  virtual void execute(){};
  virtual void finalize(){};

protected:
  // We create vectors to store our parameters (x,y,z) and NN output
  VectorPostprocessorValue & _x_values;
  VectorPostprocessorValue & _y_values;
  VectorPostprocessorValue & _z_values;
  VectorPostprocessorValue & _nn_values;
};

#endif
