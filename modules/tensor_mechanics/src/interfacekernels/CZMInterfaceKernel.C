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

template <>
InputParameters
validParams<CZMInterfaceKernel>()
{
  InputParameters params = validParams<InterfaceKernel>();
  params.addRequiredParam<unsigned int>("disp_index",
                                        "the component of the "
                                        "displacement vector this kernel is working on:"
                                        " disp_index == 0, ==> X"
                                        " disp_index == 1, ==> Y"
                                        " disp_index == 2, ==> Z");

  params.addRequiredCoupledVar("displacements", "the string containing dispalcement variables");

  params.addClassDescription(
      "Cohesive Zone Interface Kernel for cohesive zone model deping only on the disaplcment jump");

  return params;
}

CZMInterfaceKernel::CZMInterfaceKernel(const InputParameters & parameters)
  : InterfaceKernel(parameters),
    _disp_index(getParam<unsigned int>("disp_index")),
    _ndisp(coupledComponents("displacements")),
    _disp_var(_ndisp),
    _disp_neighbor_var(_ndisp),
    // residual and jacobian coefficients are material properties and represents
    // the residual and jacobain of the traction sepration law wrt the displacement jump.

    _traction(getMaterialPropertyByName<RealVectorValue>("traction")),
    _traction_derivative(getMaterialPropertyByName<RankTwoTensor>("traction_spatial_derivatives"))
{
  if (!parameters.isParamValid("boundary"))
  {
    mooseError("In order to use  CZMInterfaceKernel ,"
               " you must specify a boundary where it will live.");
  }

  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    _disp_var[i] = coupled("displacements", i);
    _disp_neighbor_var[i] = coupled("displacements", i);
  }
}

Real
CZMInterfaceKernel::computeQpResidual(Moose::DGResidualType type)
{

  Real r = _traction[_qp](_disp_index);

  switch (type)
  {
    // [test_slave-test_master]*T where T repsents the traction.
    // the + and - signs below are in accordance with this convention
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
  // retrieve the diagonal jacobain coefficient dependning on the disaplcement
  // component (_disp_index) this kernel is working on
  Real jac = _traction_derivative[_qp](_disp_index, _disp_index);

  switch (type)
  {
    // (1) and (-1) terms in parenthesis are the derivatives of \deltaU with respect to slave and
    // master variables to make the code easier to understand the trailing + and - signs are
    // inherited directly from the reidual equation
    case Moose::ElementElement:
      jac *= -_test[_i][_qp] * _phi[_j][_qp] * (-1);
      break;

    case Moose::ElementNeighbor:
      jac *= -_test[_i][_qp] * _phi_neighbor[_j][_qp] * (1);
      break;

    case Moose::NeighborElement:
      jac *= _test_neighbor[_i][_qp] * _phi[_j][_qp] * (-1);
      break;

    case Moose::NeighborNeighbor:
      jac *= _test_neighbor[_i][_qp] * _phi_neighbor[_j][_qp] * (1);
      break;
  }

  return jac;
}

Real
CZMInterfaceKernel::computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar)
{

  // set the off diag index depending on the coupled variable ID (jvar) and
  // on the displacement index this kernel is working on (_disp_index)
  std::vector<unsigned int> indeces(_ndisp, 0);
  indeces[0] = _disp_index;
  switch (_disp_index)
  {
    case (0):
      indeces[1] = 1;
      if (_ndisp == 3)
        indeces[2] = 2;
      break;
    case (1):
      indeces[1] = 0;
      if (_ndisp == 3)
        indeces[2] = 2;
      break;
    case (2):
      indeces[1] = 0;
      if (_ndisp == 3)
        indeces[2] = 1;
      break;
    default:
      mooseError("wrong _disp_index");
      break;
  }

  // retrieve the off diagonal index
  unsigned int OffDiagIndex = 3;
  // set index to a non existing values if OffDiagIndex
  // does not change a segfault error will appear
  if (jvar == _disp_var[1] || jvar == _disp_neighbor_var[1])
    OffDiagIndex = indeces[1];

  else if ((jvar == _disp_var[2] || jvar == _disp_neighbor_var[2]) && (_ndisp == 3))
    OffDiagIndex = indeces[2];

  Real jac = _traction_derivative[_qp](_disp_index, OffDiagIndex);

  switch (type)
  {
    // (1) and (-1) terms in parenthesis are the derivatives of \deltaU with respect to slave and
    // master variables
    case Moose::ElementElement:
      jac *= -_test[_i][_qp] * _phi[_j][_qp] * (-1);
      break;

    case Moose::ElementNeighbor:
      jac *= -_test[_i][_qp] * _phi_neighbor[_j][_qp] * (1);
      break;

    case Moose::NeighborElement:
      jac *= _test_neighbor[_i][_qp] * _phi[_j][_qp] * (-1);
      break;

    case Moose::NeighborNeighbor:
      jac *= _test_neighbor[_i][_qp] * _phi_neighbor[_j][_qp] * (1);
      break;

    default:
      mooseError("unknown type of jacobian");
      break;
  }
  return jac;
}
