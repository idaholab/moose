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
  InputParameters params = InterfaceKernel::validParams();
  params.addRequiredParam<unsigned int>("component",
                                        "The component of the "
                                        "displacement vector this kernel is working on:"
                                        " component == 0, ==> X"
                                        " component == 1, ==> Y"
                                        " component == 2, ==> Z");
  params.set<bool>("_use_undisplaced_reference_points") = true;

  params.addRequiredCoupledVar("displacements", "The string containing displacement variables");

  params.addClassDescription(
      "CZM Interface kernel to use when using the Total Lagrangin kinematic formulation.");
  params.suppressParameter<bool>("use_displaced_mesh");
  return params;
}

CZMInterfaceKernelTotalLagrangian::CZMInterfaceKernelTotalLagrangian(
    const InputParameters & parameters)
  : InterfaceKernel(parameters),
    _component(getParam<unsigned int>("component")),
    _ndisp(coupledComponents("displacements")),
    _disp_var(_ndisp),
    _disp_neighbor_var(_ndisp),
    _vars(_ndisp),
    _PK1traction(getMaterialPropertyByName<RealVectorValue>("PK1traction")),
    _dPK1traction_djumpglobal(getMaterialPropertyByName<RankTwoTensor>("dtraction_djump_global")),
    _dPK1traction_dF(getMaterialPropertyByName<RankThreeTensor>("dPK1traction_dF"))
{

  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    _disp_var[i] = coupled("displacements", i);
    _disp_neighbor_var[i] = coupled("displacements", i);
    _vars[i] = getVar("displacements", i);
  }
}

Real
CZMInterfaceKernelTotalLagrangian::computeQpResidual(Moose::DGResidualType type)
{

  Real r = _PK1traction[_qp](_component);

  switch (type)
  {
    // [test_slave-test_master]*T where T represents the traction.
    case Moose::Element:
      r *= -_test[_i][_qp];
      break;
    case Moose::Neighbor:
      r *= _test_neighbor[_i][_qp];
      break;
  }

  mooseAssert(std::isfinite(r), "CZMInterfaceKernelTotalLagrangian r is not finite");
  return r;
}

Real
CZMInterfaceKernelTotalLagrangian::computeQpJacobian(Moose::DGJacobianType type)
{
  // Retrieve the diagonal jacobian coefficient depennding on the displacement
  // component (_component) this kernel is working on
  Real jacsd = _dPK1traction_djumpglobal[_qp](_component, _component);
  Real jac = 0;
  switch (type)
  {
    case Moose::ElementElement: // Residual_sign -1  ddeltaU_ddisp sign -1;
      jac += _test[_i][_qp] * jacsd * _vars[_component]->phiFace()[_j][_qp];
      jac -= _test[_i][_qp] * JacLD(_component, /*neighbor=*/false);
      break;
    case Moose::ElementNeighbor: // Residual_sign -1  ddeltaU_ddisp sign 1;
      jac -= _test[_i][_qp] * jacsd * _vars[_component]->phiFaceNeighbor()[_j][_qp];
      jac -= _test[_i][_qp] * JacLD(_component, /*neighbor=*/true);
      break;
    case Moose::NeighborElement: // Residual_sign 1  ddeltaU_ddisp sign -1;
      jac -= _test_neighbor[_i][_qp] * jacsd * _vars[_component]->phiFace()[_j][_qp];
      jac += _test_neighbor[_i][_qp] * JacLD(_component, /*neighbor=*/false);
      break;
    case Moose::NeighborNeighbor: // Residual_sign 1  ddeltaU_ddisp sign 1;
      jac += _test_neighbor[_i][_qp] * jacsd * _vars[_component]->phiFaceNeighbor()[_j][_qp];
      jac += _test_neighbor[_i][_qp] * JacLD(_component, /*neighbor=*/true);
      break;
  }
  mooseAssert(std::isfinite(jac), "CZMInterfaceKernelTotalLagrangian diag jacobian is not finite");
  return jac;
}

Real
CZMInterfaceKernelTotalLagrangian::computeQpOffDiagJacobian(Moose::DGJacobianType type,
                                                            unsigned int jvar)
{

  // find the displacement component associated to jvar
  unsigned int off_diag_component;
  for (off_diag_component = 0; off_diag_component < _ndisp; off_diag_component++)
    if (_disp_var[off_diag_component] == jvar)
      break;

  mooseAssert(off_diag_component < _ndisp,
              "CZMInterfaceKernelTotalLagrangian::computeQpOffDiagJacobian wrong "
              "offdiagonal variable");

  Real jacsd = _dPK1traction_djumpglobal[_qp](_component, off_diag_component);
  Real jac = 0;

  switch (type)
  {
    case Moose::ElementElement: // Residual_sign -1  ddeltaU_ddisp sign -1;
      jac += _test[_i][_qp] * jacsd * _vars[off_diag_component]->phiFace()[_j][_qp];
      jac -= _test[_i][_qp] * JacLD(off_diag_component, /*neighbor=*/false);
      break;
    case Moose::ElementNeighbor: // Residual_sign -1  ddeltaU_ddisp sign 1;
      jac -= _test[_i][_qp] * jacsd * _vars[off_diag_component]->phiFaceNeighbor()[_j][_qp];
      jac -= _test[_i][_qp] * JacLD(off_diag_component, /*neighbor=*/true);
      break;
    case Moose::NeighborElement: // Residual_sign 1  ddeltaU_ddisp sign -1;
      jac -= _test_neighbor[_i][_qp] * jacsd * _vars[off_diag_component]->phiFace()[_j][_qp];
      jac += _test_neighbor[_i][_qp] * JacLD(off_diag_component, /*neighbor=*/false);
      break;
    case Moose::NeighborNeighbor: // Residual_sign 1  ddeltaU_ddisp sign 1;
      jac +=
          _test_neighbor[_i][_qp] * jacsd * _vars[off_diag_component]->phiFaceNeighbor()[_j][_qp];
      jac += _test_neighbor[_i][_qp] * JacLD(off_diag_component, /*neighbor=*/true);
      break;
  }
  mooseAssert(std::isfinite(jac),
              "CZMInterfaceKernelTotalLagrangian off diag jacobian is not finite");
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
