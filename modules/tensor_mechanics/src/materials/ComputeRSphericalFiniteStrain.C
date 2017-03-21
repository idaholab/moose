/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeRSphericalFiniteStrain.h"
#include "Assembly.h"
#include "FEProblem.h"
#include "MooseMesh.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<ComputeRSphericalFiniteStrain>()
{
  InputParameters params = validParams<ComputeFiniteStrain>();
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
  const auto & subdomainIDs = _mesh.meshSubdomains();
  for (auto subdomainID : subdomainIDs)
    if (_fe_problem.getCoordSystem(subdomainID) != Moose::COORD_RSPHERICAL)
      mooseError("The coordinate system must be set to RSPHERICAL for 1D R spherical simulations.");
}

void
ComputeRSphericalFiniteStrain::computeProperties()
{
  // Method from Rashid, 1993
  RankTwoTensor ave_Fhat;

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
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

    // The polar and azimuthal strains are equalivalent in this 1D problem
    A(2, 2) = A(1, 1);
    Fbar(2, 2) = Fbar(1, 1);

    // Gauss point deformation gradient
    _deformation_gradient[_qp] = A;
    _deformation_gradient[_qp].addIa(1.0);

    // very nearly A = gradU - gradUold, adapted to cylindrical coords
    A -= Fbar;

    // Fbar = ( I + gradUold)
    Fbar.addIa(1.0);

    // Incremental deformation gradient _Fhat = I + A Fbar^-1
    _Fhat[_qp] = A * Fbar.inverse();
    _Fhat[_qp].addIa(1.0);

    computeQpStrain();
  }
}
