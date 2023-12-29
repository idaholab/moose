//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeRSphericalIncrementalStrain.h"
#include "Assembly.h"
#include "FEProblem.h"
#include "MooseMesh.h"

#include "libmesh/quadrature.h"

registerMooseObject("TensorMechanicsApp", ComputeRSphericalIncrementalStrain);

InputParameters
ComputeRSphericalIncrementalStrain::validParams()
{
  InputParameters params = ComputeIncrementalSmallStrain::validParams();
  params.addClassDescription(
      "Compute a strain increment for incremental strains in 1D spherical symmetry problems.");
  return params;
}

ComputeRSphericalIncrementalStrain::ComputeRSphericalIncrementalStrain(
    const InputParameters & parameters)
  : ComputeIncrementalSmallStrain(parameters), _disp_old_0(coupledValueOld("displacements", 0))
{
}

void
ComputeRSphericalIncrementalStrain::initialSetup()
{
  ComputeIncrementalStrainBase::initialSetup();

  const auto & subdomainIDs = _mesh.meshSubdomains();
  for (auto subdomainID : subdomainIDs)
    if (_fe_problem.getCoordSystem(subdomainID) != Moose::COORD_RSPHERICAL)
      mooseError("The coordinate system must be set to RSPHERICAL for 1D R spherical simulations.");
}

void
ComputeRSphericalIncrementalStrain::computeTotalStrainIncrement(
    RankTwoTensor & total_strain_increment)
{
  // Deformation gradient calculation in cylindrical coordinates
  RankTwoTensor A;    // Deformation gradient
  RankTwoTensor Fbar; // Old Deformation gradient

  // Step through calculating the current and old deformation gradients
  // Only diagonal components are nonzero because this is a 1D material
  // Note: x_disp is the radial displacement
  A(0, 0) = (*_grad_disp[0])[_qp](0);
  Fbar(0, 0) = (*_grad_disp_old[0])[_qp](0);

  // The polar and azimuthal strains are functions of radial displacement
  if (!MooseUtils::relativeFuzzyEqual(_q_point[_qp](0), 0.0))
  {
    A(1, 1) = (*_disp[0])[_qp] / _q_point[_qp](0);
    Fbar(1, 1) = _disp_old_0[_qp] / _q_point[_qp](0);
  }

  // The polar and azimuthal strains are equivalent in this 1D problem
  A(2, 2) = A(1, 1);
  Fbar(2, 2) = Fbar(1, 1);

  // Gauss point deformation gradient
  _deformation_gradient[_qp] = A;
  _deformation_gradient[_qp].addIa(1.0);

  // very nearly A = gradU - gradUold, adapted to cylindrical coords
  A -= Fbar;

  total_strain_increment = 0.5 * (A + A.transpose());
}
