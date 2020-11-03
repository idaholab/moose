//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LevelSetMultiMaterial.h"
#include "MooseVariable.h"

registerMooseObject("XFEMApp", LevelSetMultiRealMaterial);
registerMooseObject("XFEMApp", LevelSetMultiRankTwoTensorMaterial);
registerMooseObject("XFEMApp", LevelSetMultiRankFourTensorMaterial);

registerMooseObject("XFEMApp", ADLevelSetMultiRealMaterial);
registerMooseObject("XFEMApp", ADLevelSetMultiRankTwoTensorMaterial);
registerMooseObject("XFEMApp", ADLevelSetMultiRankFourTensorMaterial);

template <typename T, bool is_ad>
InputParameters
LevelSetMultiMaterialTempl<T, is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Compute a material property for multi-materials (consisting of multiple "
      "different materials) defined by multiple level set functions.");
  params.addRequiredParam<std::vector<VariableName>>(
      "level_set_vars", "The names of the level set variables used to represent the interface");
  params.addRequiredParam<std::vector<std::string>>(
      "base_name_keys",
      "The keys of the levelset-encoding-to-base-name map. For example, if the base name should "
      "correspond to positive levelset variable 1 and negative levelset variable 2, then the "
      "encoding is '+-'.");
  params.addRequiredParam<std::vector<std::string>>(
      "base_name_vals", "The values of the levelset-encoding-to-base-name map");
  params.addParam<std::string>("base_name",
                               "Base name for the computed material property (optional)");
  params.addRequiredParam<std::string>("prop_name", "Name for the computed material property.");
  return params;
}

template <typename T, bool is_ad>
LevelSetMultiMaterialTempl<T, is_ad>::LevelSetMultiMaterialTempl(const InputParameters & parameters)
  : Material(parameters),
    _level_set_var_names(getParam<std::vector<VariableName>>("level_set_vars")),
    _nvars(_level_set_var_names.size()),
    _level_set_var_numbers(_nvars),
    _systems(_nvars),
    _solutions(_nvars),
    _keys(getParam<std::vector<std::string>>("base_name_keys")),
    _vals(getParam<std::vector<std::string>>("base_name_vals")),

    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _prop_name(getParam<std::string>("prop_name")),
    _prop(declareGenericProperty<T, is_ad>(_base_name + _prop_name))
{
  FEProblemBase * fe_problem = dynamic_cast<FEProblemBase *>(&_subproblem);
  if (!fe_problem)
    mooseError("Problem casting _subproblem to FEProblemBase");
  _xfem = MooseSharedNamespace::dynamic_pointer_cast<XFEM>(fe_problem->getXFEM());

  for (unsigned int i = 0; i < _nvars; i++)
  {
    _level_set_var_numbers[i] = _subproblem
                                    .getVariable(_tid,
                                                 _level_set_var_names[i],
                                                 Moose::VarKindType::VAR_ANY,
                                                 Moose::VarFieldType::VAR_FIELD_STANDARD)
                                    .number();
    _systems[i] = &_subproblem.getSystem(_level_set_var_names[i]);
    _solutions[i] = _systems[i]->current_local_solution.get();
  }

  for (unsigned int i = 0; i < _keys.size(); i++)
  {
    if (_keys[i].size() != _nvars)
      mooseError("The length of each base_name_key must be equal to the number of level set "
                 "variables which is ",
                 _nvars,
                 ", got key ",
                 _keys[i]);
    unsigned int key = encodeLevelSetKey(_keys[i]);
    _base_name_map.emplace(key, _vals[i]);
    _prop_map.emplace(key, &getGenericMaterialProperty<T, is_ad>(_vals[i] + "_" + _prop_name));
  }
}

template <typename T, bool is_ad>
void
LevelSetMultiMaterialTempl<T, is_ad>::computeProperties()
{
  const Node * node = pickOnePhysicalNode();

  // compute the key based on levelset variable signs
  unsigned int key = 0;
  for (unsigned int i = 0; i < _nvars; i++)
  {
    dof_id_type ls_dof_id = node->dof_number(_systems[i]->number(), _level_set_var_numbers[i], 0);
    Number ls_node_value = (*_solutions[i])(ls_dof_id);
    if (ls_node_value > 0.0)
      key += std::pow(2, i);
  }

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
    throw MooseException(name() +
                         ": Unknown hashed combination of signs of the level-set variables: " +
                         Moose::stringify(key) +
                         ", which is not "
                         "provided in the base_name_keys");
  }

  Material::computeProperties();
}

template <typename T, bool is_ad>
void
LevelSetMultiMaterialTempl<T, is_ad>::computeQpProperties()
{
  _prop[_qp] = (*_mapped_prop)[_qp];
}

template <typename T, bool is_ad>
unsigned int
LevelSetMultiMaterialTempl<T, is_ad>::encodeLevelSetKey(const std::string key)
{
  int encoded_key = 0;
  for (unsigned int i = 0; i < key.size(); i++)
    if (key[i] == '+')
      encoded_key += std::pow(2, i);
  return encoded_key;
}

template <typename T, bool is_ad>
const Node *
LevelSetMultiMaterialTempl<T, is_ad>::pickOnePhysicalNode()
{
  for (auto i : _current_elem->node_index_range())
    if (_xfem->isPointInsidePhysicalDomain(_current_elem, _current_elem->node_ref(i)))
      return _current_elem->node_ptr(i);
  mooseError("cannot find a physical node in the current element");
  return nullptr;
}

template class LevelSetMultiMaterialTempl<Real, false>;
template class LevelSetMultiMaterialTempl<RankTwoTensor, false>;
template class LevelSetMultiMaterialTempl<RankFourTensor, false>;

template class LevelSetMultiMaterialTempl<Real, true>;
template class LevelSetMultiMaterialTempl<RankTwoTensor, true>;
template class LevelSetMultiMaterialTempl<RankFourTensor, true>;
