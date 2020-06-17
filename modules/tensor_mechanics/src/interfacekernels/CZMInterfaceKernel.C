//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CZMInterfaceKernel.h"

registerMooseObject("TensorMechanicsApp", CZMInterfaceKernel);

InputParameters
CZMInterfaceKernel::validParams()
{
  InputParameters params = InterfaceKernel::validParams();
  params.addRequiredParam<unsigned int>("component",
                                        "the component of the "
                                        "displacement vector this kernel is working on:"
                                        " component == 0, ==> X"
                                        " component == 1, ==> Y"
                                        " component == 2, ==> Z");
  params.set<bool>("_use_undisplaced_reference_points") = true;

  params.addRequiredCoupledVar("displacements", "the string containing displacement variables");

  params.addClassDescription("Interface kernel for use with cohesive zone models (CZMs) that "
                             "compute traction as a function of the displacement jump");

  return params;
}

CZMInterfaceKernel::CZMInterfaceKernel(const InputParameters & parameters)
  : InterfaceKernel(parameters),
    _component(getParam<unsigned int>("component")),
    _ndisp(coupledComponents("displacements")),
    _disp_var(_ndisp),
    _disp_neighbor_var(_ndisp),
    _traction_global(getMaterialPropertyByName<RealVectorValue>("traction_global")),
    _traction_derivatives_global(
        getMaterialPropertyByName<RankTwoTensor>("traction_derivatives_global"))
{
  if (getParam<bool>("use_displaced_mesh") == true)
    mooseError("CZMInterfaceKernel cannot be used with use_displaced_mesh = true");

  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    _disp_var[i] = coupled("displacements", i);
    _disp_neighbor_var[i] = coupled("displacements", i);
  }
}

Real
CZMInterfaceKernel::computeQpResidual(Moose::DGResidualType type)
{

  Real r = _traction_global[_qp](_component);

  switch (type)
  {
    // [test_secondary-test_master]*T where T represents the traction.
    case Moose::Element:
      r *= -_test[_i][_qp];
      break;
    case Moose::Neighbor:
      r *= _test_neighbor[_i][_qp];
      break;
  }
  return r;
}

Real
CZMInterfaceKernel::computeQpJacobian(Moose::DGJacobianType type)
{
  // retrieve the diagonal Jacobian coefficient dependning on the displacement
  // component (_component) this kernel is working on
  Real jac = _traction_derivatives_global[_qp](_component, _component);

  switch (type)
  {
    case Moose::ElementElement: // Residual_sign -1  ddeltaU_ddisp sign -1;
      jac *= _test[_i][_qp] * _phi[_j][_qp];
      break;
    case Moose::ElementNeighbor: // Residual_sign -1  ddeltaU_ddisp sign 1;
      jac *= -_test[_i][_qp] * _phi_neighbor[_j][_qp];
      break;
    case Moose::NeighborElement: // Residual_sign 1  ddeltaU_ddisp sign -1;
      jac *= -_test_neighbor[_i][_qp] * _phi[_j][_qp];
      break;
    case Moose::NeighborNeighbor: // Residual_sign 1  ddeltaU_ddisp sign 1;
      jac *= _test_neighbor[_i][_qp] * _phi_neighbor[_j][_qp];
      break;
  }
  return jac;
}

Real
CZMInterfaceKernel::computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar)
{

  // find the displacement component associated to jvar
  unsigned int off_diag_component;
  for (off_diag_component = 0; off_diag_component < _ndisp; off_diag_component++)
    if (_disp_var[off_diag_component] == jvar)
      break;

  mooseAssert(off_diag_component < _ndisp,
              "CZMInterfaceKernel::computeQpOffDiagJacobian wrong offdiagonal variable");

  Real jac = _traction_derivatives_global[_qp](_component, off_diag_component);

  switch (type)
  {
    case Moose::ElementElement: // Residual_sign -1  ddeltaU_ddisp sign -1;
      jac *= _test[_i][_qp] * _phi[_j][_qp];
      break;
    case Moose::ElementNeighbor: // Residual_sign -1  ddeltaU_ddisp sign 1;
      jac *= -_test[_i][_qp] * _phi_neighbor[_j][_qp];
      break;
    case Moose::NeighborElement: // Residual_sign 1  ddeltaU_ddisp sign -1;
      jac *= -_test_neighbor[_i][_qp] * _phi[_j][_qp];
      break;
    case Moose::NeighborNeighbor: // Residual_sign 1  ddeltaU_ddisp sign 1;
      jac *= _test_neighbor[_i][_qp] * _phi_neighbor[_j][_qp];
      break;
  }
  return jac;
}
