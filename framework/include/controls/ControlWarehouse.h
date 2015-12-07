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

#ifndef CONTROLWAREHOUSE_H
#define CONTROLWAREHOUSE_H

// MOOSE includes
#include "MooseObjectWarehouse.h"
#include "Control.h"

/**
 * Non threaded storage for Control objects.
 */
class ControlWarehouse : public MooseObjectWarehouse<Control>
{
public:

  /**
   * Contrcutor.
   */
  ControlWarehouse();

  /**
   * Convienence method for calling setup methods (initialSetup, jacobianSetup, ...)
   */
  void setup(const ExecFlagType & exec_flag);

  /**
   * Call the execute methods of Control objects.
   */
  virtual void execute(const ExecFlagType & exec_flag);
};

#endif // CONTROLWAREHOUSE
