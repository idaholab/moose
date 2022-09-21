//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimeIntegrationMaterial.h"
#include "RankTwoTensor.h"

registerMooseObject("MooseApp", TimeIntegrationMaterial);
registerMooseObject("MooseApp", ADTimeIntegrationMaterial);
registerMooseObject("MooseApp", TimeIntegrationRankTwoMaterial);
registerMooseObject("MooseApp", ADTimeIntegrationRankTwoMaterial);

template <typename T, bool is_ad>
InputParameters
TimeIntegrationMaterialTempl<T, is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Time integrates a set of material properties");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "rate_prop_names", "Input material property names that will be time integrated.");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "prop_names",
      "Output material property names that will contain the integrated value of the corresponding "
      "rate_prop_names properties.");
  return params;
}

template <typename T, bool is_ad>
TimeIntegrationMaterialTempl<T, is_ad>::TimeIntegrationMaterialTempl(
    const InputParameters & parameters)
  : Material(parameters)
{
  const auto names =
      getParam<MaterialPropertyName, MaterialPropertyName>("rate_prop_names", "prop_names");
  for (const auto & name : names)
    _prop_tuples.push_back(std::make_tuple(&getGenericMaterialPropertyByName<T, is_ad>(name.first),
                                           &declareGenericProperty<T, is_ad>(name.second),
                                           &getMaterialPropertyOldByName<T>(name.second)));
}

template <typename T, bool is_ad>
void
TimeIntegrationMaterialTempl<T, is_ad>::computeQpProperties()
{
  for (auto & tuple : _prop_tuples)
    (*std::get<1>(tuple))[_qp] += (*std::get<2>(tuple))[_qp] + _dt * (*std::get<0>(tuple))[_qp];
}

template class TimeIntegrationMaterialTempl<Real, false>;
template class TimeIntegrationMaterialTempl<Real, true>;
template class TimeIntegrationMaterialTempl<RankTwoTensor, false>;
template class TimeIntegrationMaterialTempl<RankTwoTensor, true>;
