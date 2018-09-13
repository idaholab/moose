/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "LevelSetBiMaterialBase.h"
#include "AuxiliarySystem.h"
#include "MooseVariable.h"
#include "XFEM.h"

template <>
InputParameters
validParams<LevelSetBiMaterialBase>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Compute a material property for bi-materials (consisting of two "
                             "different materials) defined by a level set function.");
  params.addRequiredParam<VariableName>(
      "level_set_var", "The name of level set variable used to represent the interface");
  params.addRequiredParam<std::string>("levelset_positive_base",
                                       "Base name for the material in level set positive region.");
  params.addRequiredParam<std::string>("levelset_negative_base",
                                       "Base name for the material in level set negative region.");
  params.addParam<std::string>("base_name",
                               "Base name for the computed material property (optional)");
  params.addRequiredParam<std::string>("prop_name", "Name for the computed material property.");
  return params;
}

LevelSetBiMaterialBase::LevelSetBiMaterialBase(const InputParameters & parameters)
  : Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _prop_name(getParam<std::string>("prop_name")),
    _level_set_var_number(_subproblem
                              .getVariable(_tid,
                                           parameters.get<VariableName>("level_set_var"),
                                           Moose::VarKindType::VAR_ANY,
                                           Moose::VarFieldType::VAR_FIELD_STANDARD)
                              .number()),
    _system(_subproblem.getSystem(getParam<VariableName>("level_set_var"))),
    _solution(_system.current_local_solution.get()),
    _use_positive_property(false)
{
  FEProblemBase * fe_problem = dynamic_cast<FEProblemBase *>(&_subproblem);

  if (fe_problem == NULL)
    mooseError("Problem casting _subproblem to FEProblemBase in XFEMMaterialStateMarkerBase");

  _xfem = MooseSharedNamespace::dynamic_pointer_cast<XFEM>(fe_problem->getXFEM());
}

void
LevelSetBiMaterialBase::computeProperties()
{
  const Node * node = _current_elem->node_ptr(0);

  dof_id_type ls_dof_id = node->dof_number(_system.number(), _level_set_var_number, 0);
  Number ls_node_value = (*_solution)(ls_dof_id);

  _use_positive_property = false;

  if (_xfem->isPointInsidePhysicalDomain(_current_elem, *node))
  {
    if (ls_node_value > 0.0)
      _use_positive_property = true;
  }
  else
  {
    if (ls_node_value < 0.0)
      _use_positive_property = true;
  }

  Material::computeProperties();
}

void
LevelSetBiMaterialBase::computeQpProperties()
{
  if (_use_positive_property)
    assignQpPropertiesForLevelSetPositive();
  else
    assignQpPropertiesForLevelSetNegative();
}
