//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MULTIAPPUSEROBJECTTRANSFER_H
#define MULTIAPPUSEROBJECTTRANSFER_H

// MOOSE includes
#include "MultiAppTransfer.h"

// Forward declarations
class MultiAppUserObjectTransfer;

template <>
InputParameters validParams<MultiAppUserObjectTransfer>();

/**
 * Samples a variable's value in the Master domain at the point where
 * the MultiApp is.  Copies that value into a postprocessor in the
 * MultiApp.
 */
class MultiAppUserObjectTransfer : public MultiAppTransfer
{
public:
  MultiAppUserObjectTransfer(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual void execute() override;

protected:
  AuxVariableName _to_var_name;
  std::string _user_object_name;

  bool _displaced_target_mesh;
};

#endif // MULTIAPPVARIABLEVALUESAMPLEPOSTPROCESSORTRANSFER_H
