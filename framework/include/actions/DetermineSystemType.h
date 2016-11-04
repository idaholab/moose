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

#ifndef DETERMINESYSTEMTYPE_H
#define DETERMINESYSTEMTYPE_H

#include "MooseObjectAction.h"

class DetermineSystemType;

template <>
InputParameters validParams<DetermineSystemType>();

class DetermineSystemType : public MooseObjectAction
{
public:
  DetermineSystemType(InputParameters parameters);

  virtual void act() override;
};

#endif /* DETERMINESYSTEMTYPE_H */
