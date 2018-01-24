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

#ifndef BADADDKERNELACTION_H
#define BADADDKERNELACTION_H

#include "MooseObjectAction.h"

class BadAddKernelAction;

template <>
InputParameters validParams<BadAddKernelAction>();

/**
 * This class is for testing an error condition. It is registered
 * to add kernels but is calling the wrong method on FEProblemBase
 * to actually add the object. DO NOT COPY!
 */
class BadAddKernelAction : public MooseObjectAction
{
public:
  BadAddKernelAction(InputParameters params);

  virtual void act();
};

#endif // BADADDKERNELACTION_H
