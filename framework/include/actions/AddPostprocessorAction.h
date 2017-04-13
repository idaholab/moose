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

#ifndef ADDPOSTPROCESSORACTION_H
#define ADDPOSTPROCESSORACTION_H

#include "MooseObjectAction.h"

class AddPostprocessorAction;

template <>
InputParameters validParams<AddPostprocessorAction>();

class AddPostprocessorAction : public MooseObjectAction
{
public:
  AddPostprocessorAction(InputParameters params);

  virtual void act() override;
};

#endif // ADDPOSTPROCESSORACTION_H
