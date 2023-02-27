//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ThreadedGeneralUserObject.h"
#include "DynamicLibraryLoader.h"
#include "StepUOInterface.h"
class StepUserObject;

/**
 * Coupling user object to use Abaqus UEXTERNALDB subroutines in MOOSE
 */
class AbaqusUExternalDB : public ThreadedGeneralUserObject, StepUOInterface
{
public:
  static InputParameters validParams();

  AbaqusUExternalDB(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void initialSetup() override;
  /// the UEXTERNALDB subroutine gets called here
  virtual void execute() override;
  virtual void threadJoin(const UserObject &) override {}
  virtual void finalize() override {}

protected:
  /// function type for the external UEXTERNALDB function
  typedef void (*uexternaldb_t)(
      int * LOP, int * LRESTART, Real TIME[], Real * DTIME, int * KSTEP, int * KINC);

  // call the plugin with the supplied LOP code
  void callPlugin(int lop);

  // The plugin file name
  FileName _plugin;

  // The plugin library wrapper
  DynamicLibraryLoader _library;

  // Function pointer to the dynamically loaded function
  uexternaldb_t _uexternaldb;

  // Abaqus simulation step number to pass in
  int _aqSTEP;

  const ExecFlagType & _current_execute_on_flag;

  /// User object that determines the step number
  const StepUserObject * _step_user_object;
};
