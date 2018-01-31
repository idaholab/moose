//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INSTANTIATEPOSTPROCESSORSACTION_H
#define INSTANTIATEPOSTPROCESSORSACTION_H

// MOOSE includes
#include "Action.h"

// Forward declerations
class InstantiatePostprocessorsAction;

template <>
InputParameters validParams<InstantiatePostprocessorsAction>();

class InstantiatePostprocessorsAction : public Action
{
public:
  InstantiatePostprocessorsAction(const InputParameters & params);

  virtual void act() override;
};

#endif // INSTANTIATEPOSTPROCESSORSACTION_H
