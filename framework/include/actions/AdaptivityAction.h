//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADAPTIVITYACTION_H
#define ADAPTIVITYACTION_H

#include "Action.h"

#ifdef LIBMESH_ENABLE_AMR

class AdaptivityAction;

template <>
InputParameters validParams<AdaptivityAction>();

class AdaptivityAction : public Action
{
public:
  AdaptivityAction(InputParameters params);

  virtual void act() override;
};

#endif // LIBMESH_ENABLE_AMR

#endif // ADAPTIVITYACTION_H
