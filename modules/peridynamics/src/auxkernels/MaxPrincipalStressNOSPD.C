//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaxPrincipalStressNOSPD.h"

registerMooseObject("PeridynamicsApp", MaxPrincipalStressNOSPD);

template <>
InputParameters
validParams<MaxPrincipalStressNOSPD>()
{
  InputParameters params = validParams<AuxKernelBasePD>();
  params.addClassDescription("Class for outputing bond max principal stress value");
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;

  return params;
}

MaxPrincipalStressNOSPD::MaxPrincipalStressNOSPD(const InputParameters & parameters)
  : AuxKernelBasePD(parameters),
    _bond_status_var(_subproblem.getVariable(_tid, "bond_status")),
    _stress(getMaterialProperty<RankTwoTensor>("stress"))
{
}

Real
MaxPrincipalStressNOSPD::computeValue()
{
  std::vector<Real> eigvals(LIBMESH_DIM, 0.0);
  RankTwoTensor avg_stress = 0.5 * (_stress[0] + _stress[1]);

  if (_bond_status_var.getElementalValue(_current_elem) > 0.5)
    avg_stress.symmetricEigenvalues(eigvals);

  return eigvals[LIBMESH_DIM - 1];
}
