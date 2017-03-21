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

#ifndef ADDVECTORPOSTPROCESSORACTION_H
#define ADDVECTORPOSTPROCESSORACTION_H

#include "MooseObjectAction.h"

class AddVectorPostprocessorAction;

template <>
InputParameters validParams<AddVectorPostprocessorAction>();

class AddVectorPostprocessorAction : public MooseObjectAction
{
public:
  AddVectorPostprocessorAction(InputParameters params);

  virtual void act() override;
};

#endif // ADDVECTORPOSTPROCESSORACTION_H
