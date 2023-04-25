//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "GeneralUserObject.h"
#include "MappingInterface.h"
#include "SurrogateModelInterface.h"
#include "SurrogateModel.h"

/**
 * A user object which takes a surrogate (or just user supplied values)
 * to determine coordinates in a latent space and uses those coordinates
 * to create approximations of full solution values for given variables.
 */
class InverseMapping : public GeneralUserObject,
                       public MappingInterface,
                       public SurrogateModelInterface
{
public:
  static InputParameters validParams();

  InverseMapping(const InputParameters & params);

  void execute() override;
  void initialize() override {}
  void finalize() override {}

  void initialSetup() override;

protected:
  /// The names of the variables which serve as a container
  /// for the reconstructed solution
  const std::vector<VariableName> & _var_names_to_fill;

  /// The names of the variables in the nonlinear system whose reconstruction
  /// we are working on
  const std::vector<VariableName> & _var_names_to_reconstruct;

  /// The names of the surrogate models for each variable
  const std::vector<UserObjectName> & _surrogate_model_names;

  /// Links to the MooseVariables of the requested variables
  std::vector<MooseVariableFieldBase *> _variable_to_fill;

  /// Links to the MooseVariables from the nonlinear system whose dof numbering
  /// we need to populate the variables in (variables_to_fill)
  std::vector<const MooseVariableFieldBase *> _variable_to_reconstruct;

  /// Link to the mapping object which provides the inverse mapping function
  VariableMappingBase * _mapping;

  /// Links to the surrogate models which provide functions to determine the
  /// coordinates in the latent space
  std::vector<SurrogateModel *> _surrogate_models;

  /// Input parameters for the surrogate models. If no surrogates are given,
  /// we assume that these are the coordinates in the latent space
  const std::vector<Real> & _input_parameters;
};
