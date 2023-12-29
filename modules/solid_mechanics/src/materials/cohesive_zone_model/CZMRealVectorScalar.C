//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CZMRealVectorScalar.h"
#include "CohesiveZoneModelTools.h"

registerMooseObject("TensorMechanicsApp", CZMRealVectorScalar);

InputParameters
CZMRealVectorScalar::validParams()
{
  InputParameters params = InterfaceMaterial::validParams();
  params.addClassDescription("Compute the normal or tangent component of a vector quantity defined "
                             "on a cohesive interface.");
  params.addRequiredParam<std::string>("real_vector_value", "The vector material name");
  params.addRequiredParam<MaterialPropertyName>(
      "property_name", "Name of the material property computed by this model");
  MooseEnum directionType("Normal Tangent");
  params.addRequiredParam<MooseEnum>("direction", directionType, "the direction: Normal, Tangent");
  params.addParam<std::string>("base_name", "Material property base name");
  return params;
}

CZMRealVectorScalar::CZMRealVectorScalar(const InputParameters & parameters)
  : InterfaceMaterial(parameters),
    _base_name(isParamValid("base_name") && !getParam<std::string>("base_name").empty()
                   ? getParam<std::string>("base_name") + "_"
                   : ""),
    _direction(getParam<MooseEnum>("direction").getEnum<DirectionType>()),
    _property(declarePropertyByName<Real>(getParam<MaterialPropertyName>("property_name"))),
    _vector(getMaterialPropertyByName<RealVectorValue>(_base_name +
                                                       getParam<std::string>("real_vector_value"))),
    _czm_rotation(getMaterialPropertyByName<RankTwoTensor>(_base_name + "czm_total_rotation"))
{
}

void
CZMRealVectorScalar::computeQpProperties()
{
  const RealVectorValue normal = _czm_rotation[_qp] * RealVectorValue(1.0, 0.0, 0.0);
  switch (_direction)
  {
    case DirectionType::Normal:
      _property[_qp] =
          CohesiveZoneModelTools::computeNormalComponents(normal, _vector[_qp]) * normal;
      break;
    case DirectionType::Tangent:
      _property[_qp] =
          CohesiveZoneModelTools::computeTangentComponents(normal, _vector[_qp]).norm();
      break;
    default:
      mooseError("ScalarType type not recognized");
      break;
  }
}
