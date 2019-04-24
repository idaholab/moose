//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IntactBondsDirichletBCPD.h"

registerMooseObject("PeridynamicsApp", IntactBondsDirichletBCPD);

template <>
InputParameters
validParams<IntactBondsDirichletBCPD>()
{
  InputParameters params = validParams<NodalBC>();
  params.addClassDescription("Class to apply Dirichlet BC based on the number of intact bonds "
                             "associated with each material point.");
  params.addRequiredCoupledVar(
      "intact_bonds_variable",
      "Name of auxiliary variable for number of intact bonds at each material point");

  return params;
}

IntactBondsDirichletBCPD::IntactBondsDirichletBCPD(const InputParameters & parameters)
  : NodalBC(parameters),
    _pdmesh(dynamic_cast<PeridynamicsMesh &>(_mesh)),
    _u_old(_var.dofValuesOld()),
    _intact_bonds_val(coupledValue("intact_bonds_variable"))
{
}

Real
IntactBondsDirichletBCPD::computeQpResidual()
{
  return _u[_qp] - _u_old[_qp];
}

bool
IntactBondsDirichletBCPD::shouldApply()
{
  bool should_apply = false;
  if (_intact_bonds_val[0] < _pdmesh.dimension())
    should_apply = true;

  return should_apply;
}
