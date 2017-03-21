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

#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "MeshModifier.h"
#include "MooseEnum.h"

// libMesh includes
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
