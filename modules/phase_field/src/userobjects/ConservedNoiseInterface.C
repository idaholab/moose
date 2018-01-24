/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ConservedNoiseInterface.h"

ConservedNoiseInterface::ConservedNoiseInterface(const InputParameters & parameters)
  : ElementUserObject(parameters), _integral(0), _volume(0), _qp(0)
{
  /**
   * This call turns on Random Number generation for this object, it can be called either in
   * the constructor or in initialSetup().
   */
  setRandomResetFrequency(EXEC_TIMESTEP_END);
}
