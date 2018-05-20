
/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "DisplacementJumpBasedCohesiveInterfaceKernel.h"

registerMooseObject("TensorMechanicsApp", DisplacementJumpBasedCohesiveInterfaceKernel);

template <>
InputParameters
validParams<DisplacementJumpBasedCohesiveInterfaceKernel>()
{
  InputParameters params = validParams<InterfaceKernel>();
  params.addRequiredParam<unsigned int>("disp_index",
                                        "the component of the "
                                        "displacement vector this kernel is working on:"
                                        " disp_index == 0, ==> X"
                                        " disp_index == 1, ==> Y"
                                        " disp_index == 2, ==> Z");
  params.addCoupledVar("disp_1",
                       "Name of the variable representing the 2nd "
                       "displacement to couple on the master side. "
                       "If disp_index == 0, then disp_1 = disp_y "
                       "If disp_index == 1, then disp_1 = disp_x "
                       "If disp_index == 2, then disp_1 = disp_x");
  params.addCoupledVar("disp_1_neighbor",
                       "Name of the variable representing "
                       "the 2nd displacement to couple on the slave side. "
                       "If disp_index == 0, disp_1_neighbor = disp_y_neighbor "
                       "If disp_index == 1, disp_1_neighbor = disp_x_neighbor "
                       "If disp_index == 2, disp_1_neighbor = disp_x_neighbor");
  params.addCoupledVar("disp_2",
                       "Name of the variable representing the 3rd "
                       "displacement to couple on the master side. "
                       "If disp_index == 0, then disp_2 = disp_z "
                       "If disp_index == 1, then disp_2 = disp_z "
                       "If disp_index == 2, then disp_2 = disp_y ");
  params.addCoupledVar("disp_2_neighbor",
                       "Name of the variable representing "
                       "the 3rd displacement to couple on the slave side. "
                       "If disp_index == 0, disp_2_neighbor = disp_z_neighbor "
                       "If disp_index == 1, disp_2_neighbor = disp_z_neighbor "
                       "If disp_index == 2, disp_2_neighbor = disp_y_neighbor");
  params.addParam<std::string>("residual",
                               "Traction",
                               "The name of the "
                               "material property representing the residual coefficients");
  params.addParam<std::string>("jacobian",
                               "TractionSpatialDerivative",
                               "The name of the  material property representing"
                               " the jacobian coefficients");
  params.addClassDescription("Cohesive Zone Interface Kernel for non-stateful"
                             "cohesive laws depending only on the displacement Jump");

  return params;
}

DisplacementJumpBasedCohesiveInterfaceKernel::DisplacementJumpBasedCohesiveInterfaceKernel(
    const InputParameters & parameters)
  : InterfaceKernel(parameters),
    _disp_index(getParam<unsigned int>("disp_index")),
    _disp_1(_mesh.dimension() >= 2 ? coupledValue("disp_1") : _zero),
    _disp_1_neighbor(_mesh.dimension() >= 2 ? coupledNeighborValue("disp_1_neighbor") : _zero),
    _disp_2(_mesh.dimension() >= 3 ? coupledValue("disp_2") : _zero),
    _disp_2_neighbor(_mesh.dimension() >= 3 ? coupledNeighborValue("disp_2_neighbor") : _zero),
    _disp_1_var(coupled("disp_1")),
    _disp_1_neighbor_var(coupled("disp_1_neighbor")),
    _disp_2_var(coupled("disp_2")),
    _disp_2_neighbor_var(coupled("disp_2_neighbor")),
    // residual and jacobian coefficients are material properties and represents
    // the residual and jacobain of the traction sepration law wrt the displacement jump.
    _residual(getParam<std::string>("residual")),
    _jacobian(getParam<std::string>("jacobian"))

{
  if (!parameters.isParamValid("boundary"))
  {
    mooseError("In order to use  DisplacementJumpBasedCohesiveInterfaceKernel ,"
               " you must specify a boundary where it will live.");
  }

  _ResidualMP = &getMaterialProperty<RealVectorValue>(_residual);
  _JacobianMP = &getMaterialProperty<RankTwoTensor>(_jacobian);
}

Real
DisplacementJumpBasedCohesiveInterfaceKernel::computeQpResidual(Moose::DGResidualType type)
{

  Real r = (*_ResidualMP)[_qp](_disp_index);

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
DisplacementJumpBasedCohesiveInterfaceKernel::computeQpJacobian(Moose::DGJacobianType type)
{
  // retrieve the diagonal jacobain coefficient dependning on the disaplcement
  // component (_disp_index) this kernel is working on
  Real jac = (*_JacobianMP)[_qp](_disp_index, _disp_index);

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
DisplacementJumpBasedCohesiveInterfaceKernel::computeQpOffDiagJacobian(Moose::DGJacobianType type,
                                                                       unsigned int jvar)
{

  if (jvar != _disp_1_var && jvar != _disp_2_var && jvar != _disp_1_neighbor_var &&
      jvar != _disp_2_neighbor_var)
  {
    mooseError("wrong variable requested");
  }

  // set the off diag index depending on the coupled variable ID (jvar) and
  // on the displacement index this kernel is working on (_disp_index)
  std::vector<unsigned int> indeces(3, 0);
  indeces[0] = _disp_index;
  if (_disp_index == 0)
  {
    indeces[1] = 1;
    indeces[2] = 2;
  }
  else if (_disp_index == 1)
  {
    indeces[1] = 0;
    indeces[2] = 2;
  }
  else if (_disp_index == 2)
  {
    indeces[1] = 0;
    indeces[2] = 1;
  }

  // retrieve the off diagonal index
  unsigned int OffDiagIndex = 3;
  // set index to a non existing values if OffDiagIndex
  // does not change a segfault error will appear
  if (jvar == _disp_1_var || jvar == _disp_1_neighbor_var)
  {
    OffDiagIndex = indeces[1];
  }
  else if (jvar == _disp_2_var || jvar == _disp_2_neighbor_var)
  {
    OffDiagIndex = indeces[2];
  }
  else
  {
    mooseError("cannot determine the proper OffDiagIndex");
  }

  Real jac = (*_JacobianMP)[_qp](_disp_index, OffDiagIndex);

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
