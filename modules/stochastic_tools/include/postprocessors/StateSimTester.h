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

#ifndef STATESIMTESTER_H
#define STATESIMTESTER_H

#include "GeneralPostprocessor.h"
#include "StateSimRunner.h"

//Forward Declarations
class StateSimTester;

// libMesh forward declarations
namespace libMesh
{
class System;
class EquationSystems;
}

template<>
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

#endif //STATESIMTESTER_H
