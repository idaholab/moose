//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseApp.h"
#include "TimeStepper.h"
#include "PerfGraphInterface.h"
#include "libmesh/parallel_object.h"

class MooseApp;
/**
 * Base class for time stepping
 */
class TimeStepperSystem : public PerfGraphInterface, public libMesh::ParallelObject
{
public:
  TimeStepperSystem(MooseApp & app);
  virtual ~TimeStepperSystem();

  void addTimeStepper(const std::string & type,
                      const std::string & name,
                      const InputParameters & params);

  void createAddedTimeSteppers();

  std::shared_ptr<TimeStepper>
  createTimeStepper(const std::string & stepper_name,
                    const std::pair<std::string, InputParameters> & type_params_pair);

  std::shared_ptr<TimeStepper> getTimeStepper(const std::string & stepper_name);

  std::shared_ptr<TimeStepper> getFinalTimeStepper();

  void setFinalTimeStepperName();

private:
  /// The MooseApp that owns this system
  MooseApp & _app;

  /// Following the example of MeshGenerators. A time stepper declared using addTimeStepper(), cleared after createTimeStepper()
  /// Key is the name, pair contains the type and the params
  std::unordered_map<std::string, std::pair<std::string, InputParameters>> _time_stepper_params;

  /// Owning storage for time steppers, map of name -> time steppers
  std::map<std::string, std::shared_ptr<TimeStepper>> _time_steppers;

  std::string _final_time_stepper_name;
};
