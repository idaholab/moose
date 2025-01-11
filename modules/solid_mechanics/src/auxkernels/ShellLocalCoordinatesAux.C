//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ShellLocalCoordinatesAux.h"
#include "libmesh/utility.h"
#include "libmesh/string_to_enum.h"

registerMooseObject("SolidMechanicsApp", ShellLocalCoordinatesAux);

InputParameters
ShellLocalCoordinatesAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "This AuxKernel stores a specific component of a shell element's local coordinate "
      "vector in an auxiliary variable.");
  params.addParam<std::string>("base_name", "Mechanical property base name");

  MooseEnum property("first_local_vector second_local_vector normal_local_vector");
  params.addRequiredParam<MooseEnum>(
      "property",
      property,
      "The local axis to output: first_local_vector, second_local_vector or normal_local_vector");
  params.addRequiredParam<unsigned int>(
      "component", "The vector component of the local coordinate vector: 0, 1 or 2");

  return params;
}

ShellLocalCoordinatesAux::ShellLocalCoordinatesAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _property(getParam<MooseEnum>("property").getEnum<PropertyType>()),
    _component(getParam<unsigned int>("component"))

{
  _local_coordinates =
      &getMaterialProperty<RankTwoTensor>(_base_name + "local_transformation_t_points_0");

  if (_component > 2)
    mooseError("Invalid component: ",
               _component,
               ". The component index of a shell local vector must be 0, 1, or 2.");
}

Real
ShellLocalCoordinatesAux::computeValue()
{
  Real output_value = 0.0;

  switch (_property)
  {
    case PropertyType::first_local_vector:
      output_value = (*_local_coordinates)[_qp](0, _component);
      break;
    case PropertyType::second_local_vector:
      output_value = (*_local_coordinates)[_qp](1, _component);
      break;
    case PropertyType::normal_local_vector:
      output_value = (*_local_coordinates)[_qp](2, _component);
      break;
  }

  return output_value;
}
