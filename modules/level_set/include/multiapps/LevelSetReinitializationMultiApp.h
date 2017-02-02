/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef LEVELSETREINITIALIZATIONMULTIAPP_H
#define LEVELSETREINITIALIZATIONMULTIAPP_H

#include "MultiApp.h"

// Forward declarations
class LevelSetReinitializationMultiApp;
class LevelSetReinitializationProblem;
class Executioner;

template<>
InputParameters validParams<LevelSetReinitializationMultiApp>();

/**
 * MultiApp that performs a time reset prior to solving, this enables the level set reinitialization to
 * solve repeatedly.
 */
class LevelSetReinitializationMultiApp : public MultiApp
{
public:
  LevelSetReinitializationMultiApp(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void advanceStep() override {}
  virtual bool solveStep(Real dt, Real target_time, bool auto_advance=true) override;

protected:

  /// Access to the level set specific problem to allow for the resetTime() method to be called.
  LevelSetReinitializationProblem * _level_set_problem;

  /// Access to the Execution to call execute()
  Executioner * _executioner;

  /// The solve interval for reinitialization.
  const unsigned int & _interval;
};

#endif // LevelSetReinitializationMultiApp_H
