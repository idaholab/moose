//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PressureAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"

PressureActionBase::PressureActionBase(const InputParameters & params,
                                       const std::string & non_ad_pressure_bc_type,
                                       const std::string & ad_pressure_bc_type)
  : Action(params),
    _non_ad_pressure_bc_type(non_ad_pressure_bc_type),
    _ad_pressure_bc_type(ad_pressure_bc_type),
    _use_ad(getParam<bool>("use_automatic_differentiation")),
    _save_in_vars({getParam<std::vector<AuxVariableName>>("save_in_disp_x"),
                   getParam<std::vector<AuxVariableName>>("save_in_disp_y"),
                   getParam<std::vector<AuxVariableName>>("save_in_disp_z")}),
    _has_save_in_vars({params.isParamValid("save_in_disp_x"),
                       params.isParamValid("save_in_disp_y"),
                       params.isParamValid("save_in_disp_z")})
{
}

void
PressureActionBase::act()
{
  const auto bc_type = _use_ad ? _ad_pressure_bc_type : _non_ad_pressure_bc_type;

  std::vector<VariableName> displacements = getParam<std::vector<VariableName>>("displacements");
  // Create pressure BCs
  for (unsigned int i = 0; i < displacements.size(); ++i)
  {
    // Create unique kernel name for each of the components
    std::string bc_name = bc_type + "_" + _name + "_" + Moose::stringify(i);

    InputParameters params = _factory.getValidParams(bc_type);
    params.applyParameters(parameters());

    params.set<NonlinearVariableName>("variable") = displacements[i];
    if (_has_save_in_vars[i])
      params.set<std::vector<AuxVariableName>>("save_in") = _save_in_vars[i];

    _problem->addBoundaryCondition(bc_type, bc_name, params);

    // The three BCs can no longer be controlled independently.
    // We agreed on PR#29603 that this was not a problem
    if (isParamValid("enable"))
      connectControllableParams("enable", bc_type, bc_name, "enable");
  }
}
