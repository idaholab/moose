//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CZMInterfaceKernelTotalLagrangian.h"

registerMooseObject("TensorMechanicsApp", CZMInterfaceKernelTotalLagrangian);

InputParameters
CZMInterfaceKernelTotalLagrangian::validParams()
{
  InputParameters params = CZMInterfaceKernelBase::validParams();
  params.set<std::string>("traction_global_name") = "PK1traction";
  return params;
}

CZMInterfaceKernelTotalLagrangian::CZMInterfaceKernelTotalLagrangian(
    const InputParameters & parameters)
  : CZMInterfaceKernelBase(parameters),
    _dPK1traction_dF(getMaterialPropertyByName<RankThreeTensor>(_base_name + "dPK1traction_dF"))
{
}

Real
CZMInterfaceKernelTotalLagrangian::computeDResidualDDisplacement(
    const unsigned int & component_j, const Moose::DGJacobianType & type) const
{
  Real jacsd = _dtraction_djump_global[_qp](_component, component_j);
  Real jac = 0;

  switch (type)
  {
    case Moose::ElementElement: // Residual_sign -1  ddeltaU_ddisp sign -1;
      jac += _test[_i][_qp] * jacsd * _vars[component_j]->phiFace()[_j][_qp];
      jac -= _test[_i][_qp] * JacLD(component_j, /*neighbor=*/false);
      break;
    case Moose::ElementNeighbor: // Residual_sign -1  ddeltaU_ddisp sign 1;
      jac -= _test[_i][_qp] * jacsd * _vars[component_j]->phiFaceNeighbor()[_j][_qp];
      jac -= _test[_i][_qp] * JacLD(component_j, /*neighbor=*/true);
      break;
    case Moose::NeighborElement: // Residual_sign 1  ddeltaU_ddisp sign -1;
      jac -= _test_neighbor[_i][_qp] * jacsd * _vars[component_j]->phiFace()[_j][_qp];
      jac += _test_neighbor[_i][_qp] * JacLD(component_j, /*neighbor=*/false);
      break;
    case Moose::NeighborNeighbor: // Residual_sign 1  ddeltaU_ddisp sign 1;
      jac += _test_neighbor[_i][_qp] * jacsd * _vars[component_j]->phiFaceNeighbor()[_j][_qp];
      jac += _test_neighbor[_i][_qp] * JacLD(component_j, /*neighbor=*/true);
      break;
  }

  return jac;
}

Real
CZMInterfaceKernelTotalLagrangian::JacLD(const unsigned int cc, const bool neighbor) const
{
  Real jacld = 0;
  RealVectorValue phi;
  if (neighbor)
    phi = 0.5 * _vars[cc]->gradPhiFaceNeighbor()[_j][_qp];
  else
    phi = 0.5 * _vars[cc]->gradPhiFace()[_j][_qp];

  for (unsigned int j = 0; j < 3; j++)
    jacld += _dPK1traction_dF[_qp](_component, cc, j) * phi(j);

  mooseAssert(std::isfinite(jacld), "CZMInterfaceKernelTotalLagrangian JacLD is not finite");
  return jacld;
}
