//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

class MeshMetaDataDependenceAction : public Action
{
public:
  MeshMetaDataDependenceAction(const InputParameters & params);

  virtual void act();

private:
  const MeshGeneratorName & _generator_prefix;

  const unsigned int & _num_elements_x_prop;
  const Real & _xmin_prop;
  const Real & _xmax_prop;
};

template <>
InputParameters validParams<MeshMetaDataDependenceAction>();
