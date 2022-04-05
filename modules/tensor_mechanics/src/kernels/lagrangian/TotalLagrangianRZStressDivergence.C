//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TotalLagrangianRZStressDivergence.h"

registerMooseObject("TensorMechanicsApp", TotalLagrangianRZStressDivergence);

InputParameters
TotalLagrangianRZStressDivergence::validParams()
{
  InputParameters params = TotalLagrangianStressDivergence::validParams();

  params.addClassDescription("Total Lagrangian stress equilibrium kernel in RZ coordinates");

  return params;
}

TotalLagrangianRZStressDivergence::TotalLagrangianRZStressDivergence(
    const InputParameters & parameters)
  : TotalLagrangianStressDivergence(parameters)
{
  // Only two displacements in 2D
  if (_ndisp != 2)
    mooseError("Two displacement components should be provided, but",
               _ndisp,
               " displacement(s) are provided.");

  // Currently we don't support stabilization with axisymmetry
  // TODO: add stabilization
  if (_stabilize_strain)
    mooseError("Stabilization is not currently supported.");
}

void
TotalLagrangianRZStressDivergence::initialSetup()
{
  if (getBlockCoordSystem() != Moose::COORD_RZ)
    mooseError("The coordinate system in the Problem block must be set to RZ for axisymmetric "
               "geometries.");

  if (getBlockCoordSystem() == Moose::COORD_RZ && _fe_problem.getAxisymmetricRadialCoord() != 0)
    mooseError("rz_coord_axis=Y is the only supported option");
}

RankTwoTensor
TotalLagrangianRZStressDivergence::testGrad(unsigned int i)
{
  // R
  if (i == 0)
  {
    auto G = gradOp(i, _grad_test[_i][_qp]);
    G(2, 2) = _test[_i][_qp] / _q_point[_qp](0);
    return G;
  }
  // Z
  else if (i == 1)
    return gradOp(i, _grad_test[_i][_qp]);
  else
    mooseError("Internal error: unexpected displacement component");
}

RankTwoTensor
TotalLagrangianRZStressDivergence::trialGrad(unsigned int k)
{
  // R
  if (k == 0)
  {
    auto G = fullGrad(k, _stabilize_strain, _grad_phi[_j][_qp], RealVectorValue());
    G(2, 2) = _phi[_j][_qp] / _q_point[_qp](0);
    return G;
  }
  // Z
  else if (k == 1)
    return fullGrad(k, _stabilize_strain, _grad_phi[_j][_qp], RealVectorValue());
  else
    mooseError("Internal error: unexpected displacement component");
}
