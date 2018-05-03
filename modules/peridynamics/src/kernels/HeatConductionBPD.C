//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatConductionBPD.h"

registerMooseObject("PeridynamicsApp", HeatConductionBPD);

template <>
InputParameters
validParams<HeatConductionBPD>()
{
  InputParameters params = validParams<KernelBasePD>();
  params.addClassDescription("Class for calculating residual and Jacobian for bond-based "
                             "peridynamic heat conduction formulation");

  return params;
}

HeatConductionBPD::HeatConductionBPD(const InputParameters & parameters)
  : KernelBasePD(parameters),
    _bond_heat_flow(getMaterialProperty<Real>("bond_heat_flow")),
    _bond_dQdT(getMaterialProperty<Real>("bond_dQdT"))
{
}

void
HeatConductionBPD::computeLocalResidual()
{
  _local_re(0) = -_bond_heat_flow[0] * _bond_status_ij;
  _local_re(1) = -_local_re(0);
}

void
HeatConductionBPD::computeLocalJacobian()
{
  for (_i = 0; _i < _test.size(); ++_i)
    for (_j = 0; _j < _phi.size(); ++_j)
      _local_ke(_i, _j) += (_i == _j ? 1 : -1) * _bond_dQdT[0] * _bond_status_ij;
}
