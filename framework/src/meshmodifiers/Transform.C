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

#include "Transform.h"

#include "libmesh/mesh_modification.h"

template<>
InputParameters validParams<Transform>()
{
  MooseEnum transforms("TRANSLATE=1, ROTATE=2, SCALE=3");

  InputParameters params = validParams<MeshModifier>();
  params.addRequiredParam<MooseEnum>("transform", transforms, "The type of transformation to perform (TRANSLATE, ROTATE, SCALE)");
  params.addRequiredParam<RealVectorValue>("vector_value", "The value to use for the transformation. When using TRANSLATE or SCALE, the xyz coordinates are applied in each direction respectively. When using ROTATE, the coordinates are interpreted as phi, theta and psi.");

  return params;
}

Transform::Transform(const std::string & name, InputParameters parameters):
    MeshModifier(name, parameters),
    _transform(getParam<MooseEnum>("transform")),
    _vector_value(getParam<RealVectorValue>("vector_value"))
{
}

Transform::~Transform()
{
}

void
Transform::modify()
{
  switch(_transform)
  {
  case 1:
    MeshTools::Modification::translate(*_mesh_ptr, _vector_value(0), _vector_value(1), _vector_value(2)); break;
  case 2:
    MeshTools::Modification::rotate(*_mesh_ptr, _vector_value(0), _vector_value(1), _vector_value(2)); break;
  case 3:
    MeshTools::Modification::scale(*_mesh_ptr, _vector_value(0), _vector_value(1), _vector_value(2)); break;
  }
}
