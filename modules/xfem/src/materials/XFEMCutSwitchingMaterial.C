//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XFEMCutSwitchingMaterial.h"

registerMooseObject("XFEMApp", XFEMCutSwitchingMaterialReal);
registerMooseObject("XFEMApp", XFEMCutSwitchingMaterialRankTwoTensor);
registerMooseObject("XFEMApp", XFEMCutSwitchingMaterialRankThreeTensor);
registerMooseObject("XFEMApp", XFEMCutSwitchingMaterialRankFourTensor);

registerMooseObject("XFEMApp", ADXFEMCutSwitchingMaterialReal);
registerMooseObject("XFEMApp", ADXFEMCutSwitchingMaterialRankTwoTensor);
registerMooseObject("XFEMApp", ADXFEMCutSwitchingMaterialRankThreeTensor);
registerMooseObject("XFEMApp", ADXFEMCutSwitchingMaterialRankFourTensor);

template <typename T, bool is_ad>
InputParameters
XFEMCutSwitchingMaterialTempl<T, is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Switch the material property based on the CutSubdomainID.");
  params.addRequiredParam<UserObjectName>("geometric_cut_userobject",
                                          "The geometric cut userobject");
  params.addRequiredParam<std::vector<CutSubdomainID>>(
      "cut_subdomain_ids", "The CutSubdomainIDs that the geometric_cut_userobject may provide.");
  params.addRequiredParam<std::vector<std::string>>(
      "base_names", "The base_names for each of the cut subdomain.");
  params.addParam<std::string>("base_name",
                               "Base name to prepend for the computed material property.");
  params.addRequiredParam<std::string>("prop_name", "name of the material property to switch");
  return params;
}

template <typename T, bool is_ad>
XFEMCutSwitchingMaterialTempl<T, is_ad>::XFEMCutSwitchingMaterialTempl(
    const InputParameters & parameters)
  : Material(parameters),
    _cut(&getUserObject<GeometricCutUserObject>("geometric_cut_userobject")),
    _keys(getParam<std::vector<CutSubdomainID>>("cut_subdomain_ids")),
    _vals(getParam<std::vector<std::string>>("base_names")),
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
XFEMCutSwitchingMaterialTempl<T, is_ad>::computeProperties()
{
  CutSubdomainID key = _xfem->getCutSubdomainID(_cut, _current_elem);

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
    throw MooseException(name() + ": Unknown CutSubdomainID: " + Moose::stringify(key) +
                         ", which is not "
                         "provided in the base_name_keys");
  }

  Material::computeProperties();
}

template class XFEMCutSwitchingMaterialTempl<Real, false>;
template class XFEMCutSwitchingMaterialTempl<RankTwoTensor, false>;
template class XFEMCutSwitchingMaterialTempl<RankThreeTensor, false>;
template class XFEMCutSwitchingMaterialTempl<RankFourTensor, false>;

template class XFEMCutSwitchingMaterialTempl<Real, true>;
template class XFEMCutSwitchingMaterialTempl<RankTwoTensor, true>;
template class XFEMCutSwitchingMaterialTempl<RankThreeTensor, true>;
template class XFEMCutSwitchingMaterialTempl<RankFourTensor, true>;
