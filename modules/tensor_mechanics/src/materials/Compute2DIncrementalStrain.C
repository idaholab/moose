/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "Compute2DIncrementalStrain.h"
#include "MooseMesh.h"

#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<Compute2DIncrementalStrain>()
{
  InputParameters params = validParams<ComputeIncrementalSmallStrain>();
  params.addClassDescription("Compute strain increment for incremental strains in 2D geometries.");
  return params;
}

Compute2DIncrementalStrain::Compute2DIncrementalStrain(const InputParameters & parameters)
  : ComputeIncrementalSmallStrain(parameters), _ave_strain_zz(false)
{
  if (!_fe_problem.mesh().hasSecondOrderElements())
    _ave_strain_zz = true;
}

void
Compute2DIncrementalStrain::computeTotalStrainIncrement(RankTwoTensor & total_strain_increment)
{
  // Deformation gradient calculation for 2D problems
  // Note: x_disp is the radial displacement, y_disp is the axial displacement
  RankTwoTensor A(
      (*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]); // Deformation gradient
  RankTwoTensor Fbar((*_grad_disp_old[0])[_qp],
                     (*_grad_disp_old[1])[_qp],
                     (*_grad_disp_old[2])[_qp]); // Old Deformation gradient

  // Compute the displacement gradient (2,2) value for plane strain, generalized plane strain, or
  // axisymmetric problems
  A(2, 2) = computeGradDispZZ();
  Fbar(2, 2) = computeGradDispZZOld();

  _deformation_gradient[_qp] = A;
  _deformation_gradient[_qp].addIa(1.0);

  A -= Fbar; // very nearly A = gradU - gradUold, adapted to cylindrical coords

  total_strain_increment = 0.5 * (A + A.transpose());
}

void
Compute2DIncrementalStrain::computeProperties()
{
  Real volumetric_strain = 0.0;
  Real out_of_plane_strain = 0.0;
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    RankTwoTensor total_strain_increment;
    computeTotalStrainIncrement(total_strain_increment);

    _strain_increment[_qp] = total_strain_increment;

    if (_volumetric_locking_correction)
      volumetric_strain +=
          (total_strain_increment(0, 0) + total_strain_increment(1, 1)) * _JxW[_qp] * _coord[_qp];

    if (_ave_strain_zz)
      out_of_plane_strain += total_strain_increment(2, 2) * _JxW[_qp] * _coord[_qp];
  }
  if (_volumetric_locking_correction)
    volumetric_strain /= _current_elem_volume;

  if (_ave_strain_zz)
    out_of_plane_strain /= _current_elem_volume;

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    if (_volumetric_locking_correction)
    {
      const Real trace_2D = _strain_increment[_qp](0, 0) + _strain_increment[_qp](1, 1);
      _strain_increment[_qp](0, 0) += (volumetric_strain - trace_2D) / 2.0;
      _strain_increment[_qp](1, 1) += (volumetric_strain - trace_2D) / 2.0;
    }

    if (_ave_strain_zz)
      _strain_increment[_qp](2, 2) = out_of_plane_strain;

    _total_strain[_qp] = _total_strain_old[_qp] + _strain_increment[_qp];

    // Remove the Eigen strain increment
    subtractEigenstrainIncrementFromStrain(_strain_increment[_qp]);

    // strain rate
    if (_dt > 0)
      _strain_rate[_qp] = _strain_increment[_qp] / _dt;
    else
      _strain_rate[_qp].zero();

    // Update strain in intermediate configuration: rotations are not needed
    _mechanical_strain[_qp] = _mechanical_strain_old[_qp] + _strain_increment[_qp];
  }
}
