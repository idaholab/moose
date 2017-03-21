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
#ifndef FULLSOLVEMULTIAPP_H
#define FULLSOLVEMULTIAPP_H

#include "MultiApp.h"

// Forward declarations
class FullSolveMultiApp;
class Executioner;

template <>
InputParameters validParams<FullSolveMultiApp>();

/**
 * This type of MultiApp will completely solve itself the first time it is asked to take a step.
 *
 * Each "step" after that it will do nothing.
 */
class FullSolveMultiApp : public MultiApp
{
public:
  FullSolveMultiApp(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual bool solveStep(Real dt, Real target_time, bool auto_advance = true) override;

  virtual void advanceStep() override {}

private:
  std::vector<Executioner *> _executioners;

  /// Whether or not this MultiApp has already been solved.
  bool _solved;
};

#endif // FULLSOLVEMULTIAPP_H
