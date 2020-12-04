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

InputParameters
HeatConductionBPD::validParams()
{
  InputParameters params = PeridynamicsKernelBase::validParams();
  params.addClassDescription("Class for calculating the residual and Jacobian for the bond-based "
                             "peridynamic heat conduction formulation");

  return params;
}

HeatConductionBPD::HeatConductionBPD(const InputParameters & parameters)
  : PeridynamicsKernelBase(parameters),
    _bond_heat_flow(getMaterialProperty<Real>("bond_heat_flow")),
    _bond_dQdT(getMaterialProperty<Real>("bond_dQdT"))
{
}

void
HeatConductionBPD::computeLocalResidual()
{
  _local_re(0) = -_bond_heat_flow[0] * _bond_status;
  _local_re(1) = -_local_re(0);
}

void
HeatConductionBPD::computeLocalJacobian()
{
  for (unsigned int i = 0; i < _nnodes; ++i)
    for (unsigned int j = 0; j < _nnodes; ++j)
      _local_ke(i, j) += -1 * (i == j ? 1 : -1) * _bond_dQdT[0] * _bond_status;
}
