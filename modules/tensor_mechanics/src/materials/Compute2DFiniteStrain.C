/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "Compute2DFiniteStrain.h"
#include "MooseMesh.h"

#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<Compute2DFiniteStrain>()
{
  InputParameters params = validParams<ComputeFiniteStrain>();
  params.addClassDescription(
      "Compute a strain increment and rotation increment for finite strains in 2D geometries.");
  return params;
}

Compute2DFiniteStrain::Compute2DFiniteStrain(const InputParameters & parameters)
  : ComputeFiniteStrain(parameters), _ave_strain_zz(false)
{
  if (!_fe_problem.mesh().hasSecondOrderElements())
    _ave_strain_zz = true;
}

void
Compute2DFiniteStrain::computeProperties()
{
  RankTwoTensor ave_Fhat;
  Real ave_dfgrd_det = 0.0;
  Real ave_dfgrd_22 = 0.0;

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    // Deformation gradient calculation for 2D problems
    // Note: x_disp is the radial displacement, y_disp is the axial displacement
    RankTwoTensor A((*_grad_disp[0])[_qp],
                    (*_grad_disp[1])[_qp],
                    (*_grad_disp[2])[_qp]); // Deformation gradient
    RankTwoTensor Fbar((*_grad_disp_old[0])[_qp],
                       (*_grad_disp_old[1])[_qp],
                       (*_grad_disp_old[2])[_qp]); // Old Deformation gradient

    // Compute the displacement gradient (2,2) value for plane strain, generalized plane strain, or
    // axisymmetric problems
    A(2, 2) = computeGradDispZZ();
    Fbar(2, 2) = computeGradDispZZOld();

    // Gauss point deformation gradient
    _deformation_gradient[_qp] = A;
    _deformation_gradient[_qp].addIa(1.0);

    A -= Fbar; // very nearly A = gradU - gradUold, adapted to cylindrical coords

    Fbar.addIa(1.0); // Fbar = ( I + gradUold)

    // Incremental deformation gradient _Fhat = I + A Fbar^-1
    _Fhat[_qp] = A * Fbar.inverse();
    _Fhat[_qp].addIa(1.0);

    if (_volumetric_locking_correction)
    {
      // Calculate average _Fhat for volumetric locking correction
      ave_Fhat(0, 0) += _Fhat[_qp](0, 0) * _JxW[_qp] * _coord[_qp];
      ave_Fhat(0, 1) += _Fhat[_qp](0, 1) * _JxW[_qp] * _coord[_qp];
      ave_Fhat(1, 0) += _Fhat[_qp](1, 0) * _JxW[_qp] * _coord[_qp];
      ave_Fhat(1, 1) += _Fhat[_qp](1, 1) * _JxW[_qp] * _coord[_qp];

      // Average deformation gradient
      ave_dfgrd_det += (_deformation_gradient[_qp](0, 0) * _deformation_gradient[_qp](1, 1) -
                        _deformation_gradient[_qp](0, 1) * _deformation_gradient[_qp](1, 0)) *
                       _JxW[_qp] * _coord[_qp];
    }
    if (_ave_strain_zz)
    {
      ave_Fhat(2, 2) += _Fhat[_qp](2, 2) * _JxW[_qp] * _coord[_qp];
      ave_dfgrd_22 += _deformation_gradient[_qp](2, 2) * _JxW[_qp] * _coord[_qp];
    }
  }
  if (_volumetric_locking_correction)
  {
    // needed for volumetric locking correction
    ave_Fhat(0, 0) /= _current_elem_volume;
    ave_Fhat(0, 1) /= _current_elem_volume;
    ave_Fhat(1, 0) /= _current_elem_volume;
    ave_Fhat(1, 1) /= _current_elem_volume;
    // average deformation gradient
    ave_dfgrd_det /= _current_elem_volume;
  }
  if (_ave_strain_zz)
  {
    ave_Fhat(2, 2) /= _current_elem_volume;
    ave_dfgrd_22 /= _current_elem_volume;
  }

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    if (_volumetric_locking_correction)
    {
      // Finalize volumetric locking correction
      const Real factor =
          std::sqrt((ave_Fhat(0, 0) * ave_Fhat(1, 1) - ave_Fhat(0, 1) * ave_Fhat(1, 0)) /
                    (_Fhat[_qp](0, 0) * _Fhat[_qp](1, 1) - _Fhat[_qp](0, 1) * _Fhat[_qp](1, 0)));
      _Fhat[_qp](0, 0) *= factor;
      _Fhat[_qp](0, 1) *= factor;
      _Fhat[_qp](1, 0) *= factor;
      _Fhat[_qp](1, 1) *= factor;

      // Volumetric locking correction
      const Real factor2 = std::sqrt(
          ave_dfgrd_det / (_deformation_gradient[_qp](0, 0) * _deformation_gradient[_qp](1, 1) -
                           _deformation_gradient[_qp](0, 1) * _deformation_gradient[_qp](1, 0)));
      _deformation_gradient[_qp](0, 0) *= factor2;
      _deformation_gradient[_qp](0, 1) *= factor2;
      _deformation_gradient[_qp](1, 0) *= factor2;
      _deformation_gradient[_qp](1, 1) *= factor2;
    }
    if (_ave_strain_zz)
    {
      _Fhat[_qp](2, 2) = ave_Fhat(2, 2);
      _deformation_gradient[_qp](2, 2) = ave_dfgrd_22;
    }

    computeQpStrain();
  }
}
