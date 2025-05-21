//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidWallGasMix.h"

registerMooseObject("ThermalHydraulicsApp", SolidWallGasMix);

InputParameters
SolidWallGasMix::validParams()
{
  InputParameters params = FlowBoundaryGasMix::validParams();
  params.addClassDescription("Wall boundary condition component for FlowChannelGasMix.");
  return params;
}

SolidWallGasMix::SolidWallGasMix(const InputParameters & params) : FlowBoundaryGasMix(params) {}

void
SolidWallGasMix::addMooseObjects()
{
  // boundary flux user object
  {
    const std::string class_name = "BoundaryFluxGasMixGhostWall";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
    params.set<Real>("normal") = _normal;
    params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};
    getTHMProblem().addUserObject(class_name, _boundary_uo_name, params);
  }

  // BCs
  addWeakBCs();
}
