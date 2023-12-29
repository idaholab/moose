//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeRSphericalFiniteStrain.h"
#include "Assembly.h"
#include "FEProblem.h"
#include "MooseMesh.h"

#include "libmesh/quadrature.h"

registerMooseObject("TensorMechanicsApp", ComputeRSphericalFiniteStrain);

InputParameters
ComputeRSphericalFiniteStrain::validParams()
{
  InputParameters params = ComputeFiniteStrain::validParams();
  params.addClassDescription("Compute a strain increment and rotation increment for finite strains "
                             "in 1D spherical symmetry problems.");
  return params;
}

ComputeRSphericalFiniteStrain::ComputeRSphericalFiniteStrain(const InputParameters & parameters)
  : ComputeFiniteStrain(parameters), _disp_old_0(coupledValueOld("displacements", 0))
{
}

void
ComputeRSphericalFiniteStrain::initialSetup()
{
  ComputeIncrementalStrainBase::initialSetup();

  const auto & subdomainIDs = _mesh.meshSubdomains();
  for (auto subdomainID : subdomainIDs)
    if (_fe_problem.getCoordSystem(subdomainID) != Moose::COORD_RSPHERICAL)
      mooseError("The coordinate system must be set to RSPHERICAL for 1D R spherical simulations.");
}

void
ComputeRSphericalFiniteStrain::computeProperties()
{
  // Method from Rashid, 1993

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    // Deformation gradient calculation in spherical coordinates
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

    // very nearly A = gradU - gradUold, adapted to spherical coords
    A -= Fbar;

    // Fbar = ( I + gradUold)
    Fbar.addIa(1.0);

    // Incremental deformation gradient _Fhat = I + A Fbar^-1
    _Fhat[_qp] = A * Fbar.inverse();
    _Fhat[_qp].addIa(1.0);

    computeQpStrain();
  }
}

void
ComputeRSphericalFiniteStrain::computeQpIncrements(RankTwoTensor & total_strain_increment,
                                                   RankTwoTensor & rotation_increment)
{
  for (unsigned int i = 0; i < 3; ++i)
    total_strain_increment(i, i) = std::log(_Fhat[_qp](i, i));

  rotation_increment.setToIdentity();
}
