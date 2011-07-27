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

#ifndef ADDKERNELACTION_H
#define ADDKERNELACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "MooseObjectAction.h"

#include <string>

class AddKernelAction;

template<>
InputParameters validParams<AddKernelAction>();


class AddKernelAction : public MooseObjectAction
{
public:
  AddKernelAction(const std::string & name, InputParameters params);

  virtual void act();

private:
  bool is_kernels_action;
};

#endif // ADDKERNELACTION_H
