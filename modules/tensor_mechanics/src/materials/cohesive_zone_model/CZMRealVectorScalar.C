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
  params.addClassDescription(
      "Compute a scalar value from a RealVector defined on a cohesive zone interface");
  params.addRequiredParam<std::string>("real_vector_value", "The vector material name");
  params.addRequiredParam<MaterialPropertyName>(
      "property_name", "Name of the material property computed by this model");
  MooseEnum scalarType("Normal Tangent");
  params.addRequiredParam<MooseEnum>(
      "scalar_type", scalarType, "Type of scalar to be computed: Normal, Tangent");
  params.addParam<std::string>("base_name", "Material property base name");
  return params;
}

CZMRealVectorScalar::CZMRealVectorScalar(const InputParameters & parameters)
  : InterfaceMaterial(parameters),
    _base_name(isParamValid("base_name") && !getParam<std::string>("base_name").empty()
                   ? getParam<std::string>("base_name") + "_"
                   : ""),
    _scalar_type(getParam<MooseEnum>("scalar_type").getEnum<ScalarType>()),
    _property(declarePropertyByName<Real>(getParam<MaterialPropertyName>("property_name"))),
    _vector(getMaterialPropertyByName<RealVectorValue>(_base_name +
                                                       getParam<std::string>("real_vector_value"))),
    _czm_rotation(getMaterialPropertyByName<RankTwoTensor>(_base_name + "czm_total_rotation")),
    _unit_vector({1, 0, 0})
{
}

void
CZMRealVectorScalar::initQpStatefulProperties()
{
  _property[_qp] = 0.0;
}

void
CZMRealVectorScalar::computeQpProperties()
{
  RealVectorValue normal = _czm_rotation[_qp] * _unit_vector;
  switch (_scalar_type)
  {
    case ScalarType::Normal:
      _property[_qp] =
          CohesiveZoneModelTools::computeNormalComponents(normal, _vector[_qp]) * normal;
      break;
    case ScalarType::Tangent:
      _property[_qp] =
          CohesiveZoneModelTools::computeTangentComponents(normal, _vector[_qp]).norm();
      break;
    default:
      mooseError("ScalarType type not recognized");
      break;
  }
}
