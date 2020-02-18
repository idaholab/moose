//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialRankTwoCartesianScalar.h"
#include "RankTwoScalarTools.h"

registerMooseObject("TensorMechanicsApp", MaterialRankTwoCartesianScalar);

defineLegacyParams(MaterialRankTwoCartesianScalar);

InputParameters
MaterialRankTwoCartesianScalar::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Compute a Cartesian scalar property of a RankTwoTensor");
  params.addRequiredParam<MaterialPropertyName>("rank_two_tensor",
                                                "The rank two material tensor name");
  params.addParam<std::string>("calculation_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  params.addParam<MooseEnum>(
      "scalar_type", RankTwoScalarTools::scalarOptions(), "Type of scalar output");
  params.addRequiredParam<Point>(
      "point1", "Start point for axis used to calculate some Cartesian material tensor quantities");
  params.addRequiredParam<Point>(
      "point2", "End point for axis used to calculate some material tensor quantities");
  params.addRequiredParam<Point>("direction", "Direction vector from point1 to point 2");
  return params;
}

MaterialRankTwoCartesianScalar::MaterialRankTwoCartesianScalar(const InputParameters & parameters)
  : Material(parameters),
    _tensor(getMaterialProperty<RankTwoTensor>("rank_two_tensor")),
    _calculation_name(isParamValid("calculation_name") ? getParam<std::string>("calculation_name")
                                                       : ""),
    _calculation(declareProperty<Real>(_calculation_name)),
    _scalar_type(getParam<MooseEnum>("scalar_type")),
    _point1(parameters.get<Point>("point1")),
    _point2(parameters.get<Point>("point2")),
    _input_direction(parameters.get<Point>("direction") / parameters.get<Point>("direction").norm())
{
}
void
MaterialRankTwoCartesianScalar::initQpStatefulProperties()
{
  _calculation[_qp] = 0.0;
}

void
MaterialRankTwoCartesianScalar::computeQpProperties()
{
  unsigned int qp = _qp;

  _calculation[_qp] = RankTwoScalarTools::getQuantity(
      _tensor[qp], _scalar_type, _point1, _point2, _q_point[qp], _input_direction);
}
