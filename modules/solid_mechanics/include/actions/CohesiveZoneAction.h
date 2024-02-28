//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CohesiveZoneActionBase.h"

class CohesiveZoneAction : public CohesiveZoneActionBase
{
public:
  static InputParameters validParams();
  CohesiveZoneAction(const InputParameters & params);

  /// Method adding the proper relationship manager
  using Action::addRelationshipManagers;
  virtual void addRelationshipManagers(Moose::RelationshipManagerType input_rm_type) override;

  void act() override;

protected:
  void actOutputGeneration();
  void actOutputMatProp();

  /// adds the required interfacekernels based on the selected strain formulation
  void addRequiredCZMInterfaceKernels();

  /// adds the required interface materials based on the selected strain formulation
  void addRequiredCZMInterfaceMaterials();

  /// verifies order and family of output variables
  void verifyOrderAndFamilyOutputs();

  /// method to prepare save_in and diag_save_in inputs for the interface kernel
  void prepareSaveInInputs(std::vector<AuxVariableName> & /*save_in_names*/,
                           std::string & /*save_in_side*/,
                           const std::vector<AuxVariableName> & /*var_name_master*/,
                           const std::vector<AuxVariableName> & /*var_name_slave*/,
                           const int & /*i*/) const;

  /// method checking multiple CohesiveZoneAction block inputs
  void chekMultipleActionParameters();

  template <typename T, typename T2>
  bool setupOutput(std::string out, T table, T2 setup);

  /// the disaplcements varaible names
  std::vector<VariableName> _displacements;

  /// number of displacement components
  const unsigned int _ndisp;

  /// whether to use AD kernels and materials
  const bool _use_AD;

  /// Base name of the material system
  const std::string _base_name;

  /// Base name of the material system
  const std::vector<BoundaryName> _boundary;

  /// strain formulation
  enum class Strain
  {
    Small,
    Finite
  } _strain;

  ///@{ residual debugging
  std::vector<AuxVariableName> _save_in_master;
  std::vector<AuxVariableName> _diag_save_in_master;
  std::vector<AuxVariableName> _save_in_slave;
  std::vector<AuxVariableName> _diag_save_in_slave;
  ///@}

  /// kernel's and materials's names
  ///@{
  std::string _czm_kernel_name;
  std::string _disp_jump_provider_name;
  std::string _equilibrium_traction_calculator_name;
  ///@}

  /// output materials to generate scalar traction/jump vector quantities
  std::vector<std::string> _generate_output;
  MultiMooseEnum _material_output_order;
  MultiMooseEnum _material_output_family;

  /// set generated from the combined boundary restrictions of all CohesiveZoneAction action blocks
  std::set<BoundaryName> _boundary_name_union;

  /// set generated from the combined boundary restrictions of all CohesiveZoneAction action blocks
  std::set<Strain> _strain_formulation_union;

  /// display info
  const bool _verbose;

  /// simple method for adding the base name to a variable
  std::string addBaseName(const std::string & name) const
  {
    return _base_name.empty() ? name : _base_name + "_" + name;
  }
};

template <typename T, typename T2>
bool
CohesiveZoneAction::setupOutput(std::string out, T table, T2 setup)
{
  for (const auto & t1 : table)
  {
    // find the officially supported properties
    for (const auto & t2 : t1.second.second)
      if (t1.first + '_' + t2 == out)
      {
        const auto it = _real_vector_cartesian_component_table.find(t2);
        if (it != _real_vector_cartesian_component_table.end())
        {
          setup(it->second, t1.second.first);
          return true;
        }
        else
          mooseError("Internal error. The permitted vector shortcuts must be keys in the "
                     "'_real_vector_cartesian_component_table'.");
      }

    // check for custom properties
    auto prefix = t1.first + '_';
    if (out.substr(0, prefix.length()) == prefix)
    {
      setup(out.substr(prefix.length()), t1.second.first);
      return true;
    }
  }

  return false;
}
