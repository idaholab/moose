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

class CohesiveZoneMasterAction : public Action
{
public:
  static InputParameters validParams();
  CohesiveZoneMasterAction(const InputParameters & params);

  /// Method adding the proper relationship manager
  using Action::addRelationshipManagers;
  virtual void addRelationshipManagers(Moose::RelationshipManagerType input_rm_type) override;

  void act() override;

protected:
  /// method to prepare save_in and diag_save_in inputs for hte interface kernel
  void prepareSaveInInputs(std::vector<AuxVariableName> & /*save_in_names*/,
                           std::string & /*save_in_side*/,
                           const std::vector<AuxVariableName> & /*var_name_master*/,
                           const std::vector<AuxVariableName> & /*var_name_slave*/,
                           const int & /*i*/) const;

  /// the disaplcements varaible names
  std::vector<VariableName> _displacements;

  /// number of displacement components
  const unsigned int _ndisp;

  /// Base name of the material system
  const std::string _base_name;

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
};
