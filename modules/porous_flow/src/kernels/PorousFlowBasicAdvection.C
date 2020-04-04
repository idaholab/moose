//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowBasicAdvection.h"

registerMooseObject("PorousFlowApp", PorousFlowBasicAdvection);

InputParameters
PorousFlowBasicAdvection::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names.");
  params.addParam<unsigned int>("phase", 0, "Use the Darcy velocity of this fluid phase");
  params.addClassDescription(
      "Advective flux of a Variable using the Darcy velocity of the fluid phase");
  return params;
}

PorousFlowBasicAdvection::PorousFlowBasicAdvection(const InputParameters & parameters)
  : Kernel(parameters),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _ph(getParam<unsigned int>("phase")),
    _darcy_velocity(
        getMaterialProperty<std::vector<RealVectorValue>>("PorousFlow_darcy_velocity_qp")),
    _ddarcy_velocity_dvar(getMaterialProperty<std::vector<std::vector<RealVectorValue>>>(
        "dPorousFlow_darcy_velocity_qp_dvar")),
    _ddarcy_velocity_dgradvar(
        getMaterialProperty<std::vector<std::vector<std::vector<RealVectorValue>>>>(
            "dPorousFlow_darcy_velocity_qp_dgradvar"))
{
  if (_ph >= _dictator.numPhases())
    paramError("phase",
               "The Dictator proclaims that the maximum phase index in this simulation is ",
               _dictator.numPhases() - 1,
               " whereas you have used ",
               _ph,
               ". Remember that indexing starts at 0. The Dictator is watching you, to "
               "ensure your wellbeing.");
}

Real
PorousFlowBasicAdvection::computeQpResidual()
{
  return -_grad_test[_i][_qp] * _darcy_velocity[_qp][_ph] * _u[_qp];
}

Real
PorousFlowBasicAdvection::computeQpJacobian()
{
  const Real result = -_grad_test[_i][_qp] * _darcy_velocity[_qp][_ph] * _phi[_j][_qp];
  return result + computeQpOffDiagJacobian(_var.number());
}

Real
PorousFlowBasicAdvection::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (_dictator.notPorousFlowVariable(jvar))
    return 0.0;

  const unsigned pvar = _dictator.porousFlowVariableNum(jvar);
  Real result =
      -_grad_test[_i][_qp] * _ddarcy_velocity_dvar[_qp][_ph][pvar] * _phi[_j][_qp] * _u[_qp];
  for (unsigned j = 0; j < LIBMESH_DIM; ++j)
    result -= _grad_test[_i][_qp] *
              (_ddarcy_velocity_dgradvar[_qp][_ph][j][pvar] * _grad_phi[_j][_qp](j)) * _u[_qp];

  return result;
}
