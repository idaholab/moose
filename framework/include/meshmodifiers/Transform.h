//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "MeshModifier.h"
#include "MooseEnum.h"

#include "libmesh/vector_value.h"

class Transform;

template <>
InputParameters validParams<Transform>();

class Transform : public MeshModifier
{
public:
  Transform(const InputParameters & parameters);

protected:
  void modify() override;

  MooseEnum _transform;
  RealVectorValue _vector_value;
};

#endif /* TRANSFORM_H */
