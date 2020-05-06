//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RankTwoInvariant.h"
#include "RankTwoScalarTools.h"

registerMooseObject("TensorMechanicsApp", RankTwoInvariant);

InputParameters
RankTwoInvariant::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Compute a invariant property of a RankTwoTensor");
  params.addRequiredParam<MaterialPropertyName>("rank_two_tensor",
                                                "The rank two material tensor name");
  params.addRequiredParam<std::string>("property_name",
                                       "Name of the material property computed by this model");
  params.addParam<MooseEnum>(
      "invariant", RankTwoScalarTools::mixedInvariantComponentOptions(), "Type of scalar output");

  return params;
}

RankTwoInvariant::RankTwoInvariant(const InputParameters & parameters)
  : Material(parameters),
    _tensor(getMaterialProperty<RankTwoTensor>("rank_two_tensor")),
    _property_name(isParamValid("property_name") ? getParam<std::string>("property_name") : ""),
    _property(declareProperty<Real>(_property_name)),
    _invariant(getParam<MooseEnum>("invariant"))
{
}

void
RankTwoInvariant::initQpStatefulProperties()
{
  _property[_qp] = 0.0;
}

void
RankTwoInvariant::computeQpProperties()
{
  if (_property_name == "max_principal_stress" || _property_name == "mid_principal_stress" ||
      _property_name == "min_principal_stress")
  {
    Point dummy_direction;
    _property[_qp] =
        RankTwoScalarTools::getPrincipalComponent(_tensor[_qp], _invariant, dummy_direction);
  }
  if (_property_name != "max_principal_stress" && _property_name != "mid_principal_stress" &&
      _property_name != "min_principal_stress")
    _property[_qp] = RankTwoScalarTools::getInvariantComponent(_tensor[_qp], _invariant);
}
