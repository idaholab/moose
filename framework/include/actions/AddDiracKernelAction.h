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

#ifndef ADDDIRACKERNELACTION_H
#define ADDDIRACKERNELACTION_H

#include "MooseObjectAction.h"

class AddDiracKernelAction;

template<>
InputParameters validParams<AddDiracKernelAction>();


class AddDiracKernelAction : public MooseObjectAction
{
public:
  AddDiracKernelAction(const std::string & name, InputParameters params);

  virtual void act();

private:
  bool is_kernels_action;
};

#endif // ADDDIRACKERNELACTION_H
