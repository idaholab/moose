//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RigidBodyDisplacementBC.h"
#include "Function.h"
#include "RigidBodyKinematicsUtils.h"

registerMooseObject("SolidMechanicsApp", RigidBodyDisplacementBC);

InputParameters
RigidBodyDisplacementBC::validParams()
{
  InputParameters params = DirichletBCBase::validParams();
  params.addClassDescription(
      "Prescribes one component of the displacement of a rigid body undergoing a finite "
      "translation and rotation about a reference point, given by functions of time.");
  params.addRequiredParam<MooseEnum>(
      "component", MooseEnum("x=0 y=1 z=2"), "The displacement component to prescribe");
  params.addRequiredParam<Point>(
      "reference_point", "Point about which the body rotates, in the reference configuration");
  params.addParam<std::vector<FunctionName>>(
      "translations",
      "Functions giving the translation of the reference point, one per spatial dimension. Zero "
      "translation if omitted.");
  params.addParam<std::vector<FunctionName>>(
      "rotations",
      "Functions giving the components of the rotation vector (rotation axis times angle in "
      "radians): one function (the rotation about z) in 2D, three in 3D. No rotation if omitted.");
  return params;
}

RigidBodyDisplacementBC::RigidBodyDisplacementBC(const InputParameters & parameters)
  : DirichletBCBase(parameters),
    _component(getParam<MooseEnum>("component")),
    _reference_point(getParam<Point>("reference_point"))
{
  const unsigned int dim = _mesh.spatialDimension();
  if (dim == 1)
    mooseError("A rigid body motion requires two or three spatial dimensions");
  if (_component >= dim)
    paramError("component", "The component must be less than the spatial dimension (", dim, ")");

  const auto get_functions = [this, dim](const std::string & param, const unsigned int size)
  {
    std::vector<const Function *> functions;
    if (!isParamValid(param))
      return functions;
    const auto & names = getParam<std::vector<FunctionName>>(param);
    if (names.size() != size)
      paramError(param, "Expected ", size, " functions in ", dim, "D");
    for (const auto & name : names)
      functions.push_back(&getFunctionByName(name));
    return functions;
  };
  _translations = get_functions("translations", dim);
  _rotations = get_functions("rotations", dim == 2 ? 1 : 3);
}

Real
RigidBodyDisplacementBC::computeQpValue()
{
  RealVectorValue translation;
  for (const auto i : index_range(_translations))
    translation(i) = _translations[i]->value(_t);

  // In 2D the single rotation function gives the rotation about z
  RealVectorValue theta;
  const unsigned int offset = 3 - _rotations.size();
  for (const auto i : index_range(_rotations))
    theta(offset + i) = _rotations[i]->value(_t);

  return RigidBodyKinematicsUtils::rigidDisplacement(
      translation, theta, *_current_node, _reference_point)(_component);
}
