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

#ifndef EARLYDELETEMULTIAPP_H
#define EARLYDELETEMULTIAPP_H

#include "MultiApp.h"

// Forward declarations
class EarlyDeleteMultiApp;
class Executioner;

template<>
InputParameters validParams<EarlyDeleteMultiApp>();

/**
 * This Multiapp simply prints to the console when it's deleted.
 */
class EarlyDeleteMultiApp :
  public MultiApp
{
public:
  EarlyDeleteMultiApp(const InputParameters & parameters);

  virtual ~EarlyDeleteMultiApp() { _console << "******* Early delete multiapp \"" << name() << "\" deleted! *******" << std::endl; }

  // Dummy overrides
  virtual bool solveStep(Real /*dt*/, Real /*target_time*/, bool /*auto_advance=true*/) override { return true; }
  virtual void advanceStep() override {}

};

#endif // EARLYDELETEMULTIAPP_H
