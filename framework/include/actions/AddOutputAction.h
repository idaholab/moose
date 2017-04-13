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

#ifndef ADDOUTPUTACTION_H
#define ADDOUTPUTACTION_H

// MOOSE includes
#include "MooseObjectAction.h"

// Forward declerations
class AddOutputAction;

template <>
InputParameters validParams<AddOutputAction>();

/**
 * Action for creating output objects
 */
class AddOutputAction : public MooseObjectAction
{
public:
  /**
   * Class constructor
   */
  AddOutputAction(InputParameters params);

  virtual void act() override;
};

#endif // ADDOUTPUTACTION_H
