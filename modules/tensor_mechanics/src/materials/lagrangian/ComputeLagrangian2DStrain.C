//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLagrangian2DStrain.h"

InputParameters
ComputeLagrangian2DStrain::validParams()
{
  InputParameters params = ComputeLagrangianStrain::validParams();
  params.addClassDescription("Compute finite strains in 2D geometries.");

  MooseEnum outOfPlaneDirection("x y z", "z");
  params.addParam<MooseEnum>(
      "out_of_plane_direction", outOfPlaneDirection, "The direction of the out-of-plane strain.");

  return params;
}

ComputeLagrangian2DStrain::ComputeLagrangian2DStrain(const InputParameters & parameters)
  : ComputeLagrangianStrain(parameters),
    _out_of_plane_direction(getParam<MooseEnum>("out_of_plane_direction"))
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
ComputeLagrangian2DStrain::initialSetup()
{
  // Setup displacements/zeros
  for (unsigned int i = 0; i < 3; i++)
  {
    // In-plane displacements
    if (i != _out_of_plane_direction)
    {
      _disp[i] = &coupledValue("displacements", i);
      _grad_disp[i] = &coupledGradient("displacements", i);
    }
    // Out-of-plane displacement is assumed to be zero
    else
    {
      _disp[i] = &_zero;
      _grad_disp[i] = &_grad_zero;
    }
  }
}

void
ComputeLagrangian2DStrain::calculateDeformationGradient()
{
  // Calculate the base deformation gradient at each qp
  // F = I + A
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    // Gradients of displacements. The out-of-plane row/column will be zero.
    _unstabilized_def_grad[_qp] = RankTwoTensor::initializeFromRows(
        (*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]);
    // Add the out-of-plane contribution
    _unstabilized_def_grad[_qp](_out_of_plane_direction, _out_of_plane_direction) =
        computeOutOfPlaneGradDisp();
    _unstabilized_def_grad[_qp].addIa(1);
  }

  // If stabilization is on do the volumetric correction
  if (_stabilize_strain)
  {
    // TODO
  }
  // If not stabilized just copy over
  else
  {
    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    {
      _avg_def_grad[_qp] = _unstabilized_def_grad[_qp];
      _def_grad[_qp] = _unstabilized_def_grad[_qp];
    }
  }
}
