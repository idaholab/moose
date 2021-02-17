//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeometricCutSwitchingMaterial.h"

registerMooseObject("XFEMApp", GeometricCutSwitchingMaterialReal);
registerMooseObject("XFEMApp", GeometricCutSwitchingMaterialRankTwoTensor);
registerMooseObject("XFEMApp", GeometricCutSwitchingMaterialRankThreeTensor);
registerMooseObject("XFEMApp", GeometricCutSwitchingMaterialRankFourTensor);

registerMooseObject("XFEMApp", ADGeometricCutSwitchingMaterialReal);
registerMooseObject("XFEMApp", ADGeometricCutSwitchingMaterialRankTwoTensor);
registerMooseObject("XFEMApp", ADGeometricCutSwitchingMaterialRankThreeTensor);
registerMooseObject("XFEMApp", ADGeometricCutSwitchingMaterialRankFourTensor);

template <typename T, bool is_ad>
InputParameters
GeometricCutSwitchingMaterialTempl<T, is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Switch the material property based on the GeometricCutSubdomainID.");
  params.addRequiredParam<UserObjectName>("geometric_cut_userobject",
                                          "The geometric cut userobject");
  params.addRequiredParam<std::vector<GeometricCutSubdomainID>>(
      "base_name_keys",
      "The keys of the base_name map. Keys are GeometricCutSubdomainIDs set by the geometric cut "
      "userobject.");
  params.addRequiredParam<std::vector<std::string>>(
      "base_name_vals",
      "The values of the base_name map. Values are base_names for each of the material property.");
  params.addParam<std::string>("base_name",
                               "Base name to prepend for the computed material property.");
  params.addRequiredParam<std::string>("prop_name", "name of the material property to switch");
  return params;
}

template <typename T, bool is_ad>
GeometricCutSwitchingMaterialTempl<T, is_ad>::GeometricCutSwitchingMaterialTempl(
    const InputParameters & parameters)
  : Material(parameters),
    _cut(&getUserObject<GeometricCutUserObject>("geometric_cut_userobject")),
    _keys(getParam<std::vector<GeometricCutSubdomainID>>("base_name_keys")),
    _vals(getParam<std::vector<std::string>>("base_name_vals")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _prop_name(getParam<std::string>("prop_name")),
    _prop(declareGenericProperty<T, is_ad>(_base_name + _prop_name))
{
  FEProblemBase * fe_problem = dynamic_cast<FEProblemBase *>(&_subproblem);
  if (!fe_problem)
    mooseError("Problem casting _subproblem to FEProblemBase");
  _xfem = MooseSharedNamespace::dynamic_pointer_cast<XFEM>(fe_problem->getXFEM());
  if (!_xfem)
    mooseError(name(), " should be used together with XFEM.");

  if (_keys.size() != _vals.size())
    mooseError("The number of base_name_keys must be equal to the number of base_name_vals. ",
               _keys.size(),
               " keys are provided, but ",
               _vals.size(),
               " values are provided.");
  for (unsigned int i = 0; i < _keys.size(); i++)
    _prop_map.emplace(_keys[i], &getGenericMaterialProperty<T, is_ad>(_vals[i] + "_" + _prop_name));
}

template <typename T, bool is_ad>
void
GeometricCutSwitchingMaterialTempl<T, is_ad>::computeProperties()
{
  GeometricCutSubdomainID key = _xfem->getGeometricCutSubdomainID(_cut, _current_elem);

  // We may run into situations where the key doesn't exist in the base_name_map. This may happen
  // when the problem is not well defined, or the level-set variables are very nonlinear, so that
  // the combination of levelset signs are not provided. Cutting the timestep may help, so we throw
  // an exception in this case.
  try
  {
    _mapped_prop = _prop_map.at(key);
  }
  catch (std::out_of_range &)
  {
    throw MooseException(name() + ": Unknown GeometricCutSubdomainID: " + Moose::stringify(key) +
                         ", which is not "
                         "provided in the base_name_keys");
  }

  Material::computeProperties();
}

template class GeometricCutSwitchingMaterialTempl<Real, false>;
template class GeometricCutSwitchingMaterialTempl<RankTwoTensor, false>;
template class GeometricCutSwitchingMaterialTempl<RankThreeTensor, false>;
template class GeometricCutSwitchingMaterialTempl<RankFourTensor, false>;

template class GeometricCutSwitchingMaterialTempl<Real, true>;
template class GeometricCutSwitchingMaterialTempl<RankTwoTensor, true>;
template class GeometricCutSwitchingMaterialTempl<RankThreeTensor, true>;
template class GeometricCutSwitchingMaterialTempl<RankFourTensor, true>;
