//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"
#include "Registry.h"
#include "Conversion.h"
#include "SerialAccess.h"
#include "RankTwoTensorForward.h"
#include "RankFourTensorForward.h"
#include "libmesh/fe_type.h"

// created types
#include "InterpolatedStatefulMaterial.h"
#include "ProjectedStatefulMaterialAux.h"
#include "ProjectedStatefulMaterialNodalPatchRecovery.h"

/**
 * Set up AuxKernels and AuxVariables for projected material property storage (PoMPS).
 */
class ProjectedStatefulMaterialStorageAction : public Action
{
public:
  static InputParameters validParams();

  ProjectedStatefulMaterialStorageAction(const InputParameters & parameters);

  virtual void act() override;

  using Action::addRelationshipManagers;
  virtual void addRelationshipManagers(Moose::RelationshipManagerType input_rm_type) override;

  /// List of supported raw types (equivalent AD types are also supported)
  typedef Moose::TypeList<Real, RealVectorValue, RankTwoTensor, RankFourTensor> SupportedTypes;

  /// get an enum with all supported types
  static MooseEnum getTypeEnum();

  template <typename T, bool is_ad>
  void processProperty(const MaterialPropertyName & prop_name);

protected:
  template <typename T, int I>
  struct ProcessProperty
  {
    static void apply(ProjectedStatefulMaterialStorageAction * self,
                      const MaterialPropertyName & prop_name)
    {
      self->processProperty<T, false>(prop_name);
      self->processProperty<T, true>(prop_name);
    }
  };

  template <typename... Ts>
  static MooseEnum getTypeEnum(typename Moose::TypeList<Ts...>);

  /// Property names for which we will do stateful material property projection
  const std::vector<MaterialPropertyName> & _prop_names;

  /// Variable order to project the properties into
  const MooseEnum _order;

  /// FEType for the variables to project the properties into
  FEType _fe_type;

  /// MooseObject name for the underlying variable type
  const std::string _var_type;
};

template <typename... Ts>
MooseEnum
ProjectedStatefulMaterialStorageAction::getTypeEnum(Moose::TypeList<Ts...>)
{
  return MooseEnum(Moose::stringify(std::vector<std::string>{typeid(Ts).name()...}, " "));
}

template <typename T, bool is_ad>
void
ProjectedStatefulMaterialStorageAction::processProperty(const MaterialPropertyName & prop_name)
{
  // check if a property of type T exists
  const auto & block_data = _problem->getMaterialData(Moose::BLOCK_MATERIAL_DATA);
  const auto & boundary_data = _problem->getMaterialData(Moose::BOUNDARY_MATERIAL_DATA);
  if (!block_data.haveGenericProperty<T, is_ad>(prop_name) &&
      !boundary_data.haveGenericProperty<T, is_ad>(prop_name))
    return;

  // number of scalar components
  const auto size = Moose::SerialAccess<T>::size();

  // generate variable names
  std::vector<VariableName> vars;
  for (const auto i : make_range(size))
    vars.push_back("_var_" + prop_name + '_' + Moose::stringify(i));

  if (_current_task == "setup_projected_properties")
  {
    // add the AuxVars for storage
    for (const auto & var : vars)
    {
      auto params = _factory.getValidParams(_var_type);
      params.applyParameters(parameters());
      params.set<std::vector<OutputName>>("outputs") = {"none"};
      _problem->addAuxVariable(_var_type, var, params);
    }

    // add material
    {
      const auto type = Registry::getClassName<InterpolatedStatefulMaterialTempl<T>>();
      auto params = _factory.getValidParams(type);
      params.applySpecificParameters(parameters(), {"block"});
      params.template set<std::vector<VariableName>>("old_state") = vars;
      params.template set<MaterialPropertyName>("prop_name") = prop_name;
      _problem->addMaterial(type, "_mat_" + prop_name, params);
    }

    // use nodal patch recovery for lagrange
    if (_fe_type.family == LAGRANGE)
    {
      // nodal variables require patch recovery (add user object)
      const auto & type =
          Registry::getClassName<ProjectedStatefulMaterialNodalPatchRecoveryTempl<T, is_ad>>();
      auto params = _factory.getValidParams(type);
      params.applySpecificParameters(parameters(), {"block"});
      params.template set<MaterialPropertyName>("property") = prop_name;
      params.template set<MooseEnum>("patch_polynomial_order") = _order;
      params.template set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
      params.template set<bool>("force_preaux") = true;
      _problem->addUserObject(type, "_npruo_" + prop_name, params);
    }
  }

  if (_current_task == "add_aux_kernel")
  {
    // create variables
    std::vector<std::string> auxnames;
    for (const auto i : make_range(size))
      auxnames.push_back("_aux_" + prop_name + '_' + Moose::stringify(i));

    // use nodal patch recovery for lagrange
    if (_fe_type.family == LAGRANGE)
    {
      // nodal variables require patch recovery (add aux kernel)
      const auto & type = "ProjectedMaterialPropertyNodalPatchRecoveryAux";
      for (const auto i : make_range(size))
      {
        auto params = _factory.getValidParams(type);
        params.applySpecificParameters(parameters(), {"block"});
        params.template set<AuxVariableName>("variable") = vars[i];
        params.template set<unsigned int>("component") = i;
        params.template set<UserObjectName>("nodal_patch_recovery_uo") = "_npruo_" + prop_name;
        params.template set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
        _problem->addAuxKernel(type, auxnames[i], params);
      }
    }
    else
    {
      // elemental variables
      const auto & type = Registry::getClassName<ProjectedStatefulMaterialAuxTempl<T, is_ad>>();
      for (const auto i : make_range(size))
      {
        auto params = _factory.getValidParams(type);
        params.applySpecificParameters(parameters(), {"block"});
        params.template set<AuxVariableName>("variable") = vars[i];
        params.template set<unsigned int>("component") = i;
        params.template set<MaterialPropertyName>("prop") = prop_name;
        params.template set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
        _problem->addAuxKernel(type, auxnames[i], params);
      }
    }
  }
}
