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
#ifndef MOOSEUNITAPP_H
#define MOOSEUNITAPP_H

#include "MooseApp.h"

class MooseUnitApp;

template <>
InputParameters validParams<MooseUnitApp>();

class MooseUnitApp : public MooseApp
{
public:
  MooseUnitApp(const InputParameters & parameters);
  virtual ~MooseUnitApp();
};

#endif /* MOOSEUNITAPP_H */
