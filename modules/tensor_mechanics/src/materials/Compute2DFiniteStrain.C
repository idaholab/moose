/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "Compute2DFiniteStrain.h"

// libmesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<Compute2DFiniteStrain>()
{
  InputParameters params = validParams<ComputeFiniteStrain>();
  params.addClassDescription("Compute a strain increment and rotation increment for finite strains in 2D geometries.");
  return params;
}

Compute2DFiniteStrain::Compute2DFiniteStrain(const InputParameters & parameters) :
    ComputeFiniteStrain(parameters)
{
}

void
Compute2DFiniteStrain::computeProperties()
{
  // Method from Rashid, 1993
  RankTwoTensor ave_Fhat;
  Real ave_dfgrd_det = 0.0;

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    // Deformation gradient calculation in cylinderical coordinates
    RankTwoTensor A;    // Deformation gradient
    RankTwoTensor Fbar; // Old Deformation gradient

    // Step through calculating the current and old deformation gradients
    // Note: x_disp is the radial displacement, y_disp is the axial displacement
    for (unsigned int j = 0; j < 2; ++j)
    {
      A(0,j) = (*_grad_disp[0])[_qp](j);
      Fbar(0,j) = (*_grad_disp_old[0])[_qp](j);
      A(1,j) = (*_grad_disp[1])[_qp](j);
      Fbar(1,j) = (*_grad_disp_old[1])[_qp](j);
    }
    A(2,2) = computeDeformGradZZ();
    Fbar(2,2) = computeDeformGradZZold();

    // Gauss point deformation gradient
    _deformation_gradient[_qp] = A;
    _deformation_gradient[_qp].addIa(1.0);

    A -= Fbar; //very nearly A = gradU - gradUold, adapted to cylinderical coords

    Fbar.addIa(1.0); //Fbar = ( I + gradUold)

    //Incremental deformation gradient _Fhat = I + A Fbar^-1
    _Fhat[_qp] = A * Fbar.inverse();
    _Fhat[_qp].addIa(1.0);

    // Calculate average _Fhat for volumetric locking correction
    ave_Fhat += _Fhat[_qp] * _JxW[_qp] * _coord[_qp];

    // Average deformation gradient
    ave_dfgrd_det += _deformation_gradient[_qp].det() * _JxW[_qp] * _coord[_qp];
  }

  // needed for volumetric locking correction
  ave_Fhat /= _current_elem_volume;
  // average deformation gradient
  ave_dfgrd_det /=_current_elem_volume;

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    // Finalize volumetric locking correction
    _Fhat[_qp] *= std::pow(ave_Fhat.det() / _Fhat[_qp].det(), 1.0/3.0);

    computeQpStrain();

    // Volumetric locking correction
    _deformation_gradient[_qp] *= std::pow(ave_dfgrd_det / _deformation_gradient[_qp].det(), 1.0/3.0);
  }
}
