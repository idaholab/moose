//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PhysicsFlowBoundary.h"

/**
 * Boundary condition where the user can specify a combination of fluxes and variable values
 */
class PhysicsGeneralFlowBoundary : public PhysicsFlowBoundary
{
public:
  static InputParameters validParams();

  PhysicsGeneralFlowBoundary(const InputParameters & params);

  virtual bool isReversible() const override { return _reversible; }

protected:
  virtual void check() const override;
  virtual void init() override;

  /// Vector of the variables for which the boundary value is being set
  std::vector<VariableName> _specified_values_variables;
  /// Vector of the functors setting the variable boundary values
  std::vector<MooseFunctorName> _specified_values_functors;
  /// Vector of the variables for which the boundary flux is being set
  std::vector<VariableName> _specified_fluxes_variables;
  /// Vector of the functors setting the boundary fluxes
  std::vector<MooseFunctorName> _specified_fluxes_functors;

  /// Map from the variables to the functors providing the boundary values
  std::map<VariableName, MooseFunctorName> _specified_values;
  /// Map from the variables (equations) to the functors providing the boundary fluxes
  std::map<VariableName, MooseFunctorName> _specified_fluxes;

  /// Keeps track of the boundary conditions used (to error if one is not)
  mutable std::vector<VariableName> _bcs_used;

  bool isValueBoundary(const VariableName & var_name) const override
  {
    return _specified_values.count(var_name);
  }
  bool isFluxBoundary(const VariableName & var_name) const override
  {
    return _specified_fluxes.count(var_name);
  }
  MooseFunctorName getBoundaryValue(const VariableName & var_name) const override
  {
    _bcs_used.push_back(var_name);
    return libmesh_map_find(_specified_values, var_name);
  }
  MooseFunctorName getBoundaryFlux(const VariableName & var_name) const override
  {
    _bcs_used.push_back(var_name);
    return libmesh_map_find(_specified_fluxes, var_name);
  }

  /// True to allow the flow to reverse, otherwise false
  bool _reversible;
};
