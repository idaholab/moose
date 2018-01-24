//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef STATESIMTESTER_H
#define STATESIMTESTER_H

#include "GeneralPostprocessor.h"
#include "StateSimRunner.h"

// Forward Declarations
class StateSimTester;

// libMesh forward declarations
namespace libMesh
{
class System;
class EquationSystems;
}

template <>
InputParameters validParams<StateSimTester>();

class StateSimTester : public GeneralPostprocessor
{
public:
  StateSimTester(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual Real getValue() override;

protected:
  enum SystemEnum
  {
    SYNCTIMES
  };

  const StateSimRunner & _state_sim_runner_ptr;
  const SystemEnum _test_val_enum;
};

#endif // STATESIMTESTER_H
