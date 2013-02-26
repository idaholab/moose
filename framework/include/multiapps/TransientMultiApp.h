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
#ifndef TRANSIENTMULTIAPP_H
#define TRANSIENTMULTIAPP_H

#include "MultiApp.h"
#include "MooseApp.h"
#include "Transient.h"
#include "TransientInterface.h"

class TransientMultiApp;

template<>
InputParameters validParams<TransientMultiApp>();

/**
 * MultiApp Implementation for Transient Apps.
 * In particular, this is important because TransientMultiApps
 * will be taken into account in the time step selection process.
 */
class TransientMultiApp :
  public MultiApp,
  public TransientInterface
{
public:
  TransientMultiApp(const std::string & name, InputParameters parameters);

  ~TransientMultiApp();

  /**
   * Advance all of the apps one timestep.
   */
  void solveStep();

  /**
   * Finds the smallest dt from among any of the apps.
   */
  Real computeDT();

private:
  std::vector<Transient *> _transient_executioners;
};

#endif // TRANSIENTMULTIAPP_H
