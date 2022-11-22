//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HFEMTrialJump.h"

registerMooseObject("MooseApp", HFEMTrialJump);

InputParameters
HFEMTrialJump::validParams()
{
  InputParameters params = DGKernel::validParams();
  params.addRequiredCoupledVar("interior_variable", "interior variable to find jumps in");
  params.addClassDescription("Imposes constraints for HFEM with side-discontinuous variables.");
  return params;
}

HFEMTrialJump::HFEMTrialJump(const InputParameters & parameters)
  : DGKernel(parameters),
    _interior_var(*getVar("interior_variable", 0)),
    _interior(_is_implicit ? _interior_var.sln() : _interior_var.slnOld()),
    _interior_neighbor(_is_implicit ? _interior_var.slnNeighbor() : _interior_var.slnOldNeighbor()),
    _phi_interior(_interior_var.phiFace()),
    _phi_interior_neighbor(_assembly.phiFaceNeighbor(_interior_var)),
    _test_interior(_interior_var.phiFace()),
    _interior_id(coupled("interior_variable"))
{
}

Real
HFEMTrialJump::computeQpResidual(Moose::DGResidualType type)
{
  // Use normal vector at qp 0 to make solution depend on geometry
  // (well, geometry and quadrature rule, with curved boundaries...)
  // rather than element numbering
  const Real sign = (_normals[0] > Point()) ? 1 : -1;

  switch (type)
  {
    case Moose::Element:
      return sign * _test[_i][_qp] * (_interior_neighbor[_qp] - _interior[_qp]);
      break;

    case Moose::Neighbor:
      return 0;
      break;
  }
  return 0;
}

Real
HFEMTrialJump::computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar)
{
  if (jvar != _interior_id)
    return 0;

  const Real sign = (_normals[0] > Point()) ? 1 : -1;

  switch (type)
  {
    case Moose::ElementElement:
      return -sign * _test[_i][_qp] * _phi_interior[_j][_qp];

    case Moose::NeighborElement:
      return 0;

    case Moose::ElementNeighbor:
      return sign * _test[_i][_qp] * _phi_interior_neighbor[_j][_qp];

    case Moose::NeighborNeighbor:
      return 0;

    default:
      break;
  }

  return 0;
}
