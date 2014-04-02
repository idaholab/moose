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
#include "MooseApp.h"
#include "Transient.h"
#include "TransientInterface.h"

class FullSolveMultiApp;

template<>
InputParameters validParams<FullSolveMultiApp>();

/**
 * This type of MultiApp will completely solve itself the first time it is asked to take a step.
 *
 * Each "step" after that it will do nothing.
 */
class FullSolveMultiApp :
  public MultiApp
{
public:
  FullSolveMultiApp(const std::string & name, InputParameters parameters);

  virtual ~FullSolveMultiApp();

  virtual void init();

  /**
   * Completely solve all of the Apps
   */
  virtual void solveStep(Real dt, Real target_time, bool auto_advance=true);

  /**
   * Actually advances time and causes output.
   */
  virtual void advanceStep(){}

private:
  std::vector<Executioner *> _executioners;

  /// Whether or not this MultiApp has already been solved.
  bool _solved;
};

#endif // FULLSOLVEMULTIAPP_H
