/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "Compute1DFiniteStrain.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<Compute1DFiniteStrain>()
{
  InputParameters params = validParams<ComputeFiniteStrain>();
  params.addClassDescription("Compute strain increment for finite strain in 1D problem");

  return params;
}

Compute1DFiniteStrain::Compute1DFiniteStrain(const InputParameters & parameters)
  : ComputeFiniteStrain(parameters)
{
}

void
Compute1DFiniteStrain::computeProperties()
{
  // RankTwoTensor ave_Fhat;
  // Real ave_dfgrd_det = 0.0;

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    RankTwoTensor A((*_grad_disp[0])[_qp],
                    (*_grad_disp[1])[_qp],
                    (*_grad_disp[2])[_qp]); // Deformation gradient
    RankTwoTensor Fbar((*_grad_disp_old[0])[_qp],
                       (*_grad_disp_old[1])[_qp],
                       (*_grad_disp_old[2])[_qp]); // Old Deformation gradient

    // Compute the displacement gradient dUy/dy and dUz/dz value for 1D problems
    A(1, 1) = computeGradDispYY();
    A(2, 2) = computeGradDispZZ();

    Fbar(1, 1) = computeGradDispYYOld();
    Fbar(2, 2) = computeGradDispZZOld();

    // Gauss point deformation gradient
    _deformation_gradient[_qp] = A;
    _deformation_gradient[_qp].addIa(1.0);

    A -= Fbar; // very nearly A = gradU - gradUold, adapted to cylindrical coords

    Fbar.addIa(1.0); // Fbar = ( I + gradUold)

    // Incremental deformation gradient _Fhat = I + A Fbar^-1
    _Fhat[_qp] = A * Fbar.inverse();
    _Fhat[_qp].addIa(1.0);

    // // Calculate average _Fhat for volumetric locking correction
    // ave_Fhat += _Fhat[_qp] * _JxW[_qp] * _coord[_qp];

    // // Average deformation gradient
    // ave_dfgrd_det += _deformation_gradient[_qp].det() * _JxW[_qp] * _coord[_qp];
  }

  // // needed for volumetric locking correction
  // ave_Fhat /= _current_elem_volume;
  // // average deformation gradient
  // ave_dfgrd_det /=_current_elem_volume;

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    // // Finalize volumetric locking correction
    // _Fhat[_qp] *= std::cbrt(ave_Fhat.det() / _Fhat[_qp].det());

    computeQpStrain();

    // // Volumetric locking correction
    // _deformation_gradient[_qp] *= std::cbrt(ave_dfgrd_det / _deformation_gradient[_qp].det());
  }
}
