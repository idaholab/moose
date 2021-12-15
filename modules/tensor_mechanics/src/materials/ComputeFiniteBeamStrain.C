//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeFiniteBeamStrain.h"
#include "Assembly.h"
#include "NonlinearSystem.h"
#include "MooseVariable.h"

#include "libmesh/quadrature.h"
#include "libmesh/utility.h"

registerMooseObject("TensorMechanicsApp", ComputeFiniteBeamStrain);

InputParameters
ComputeFiniteBeamStrain::validParams()
{
  InputParameters params = ComputeIncrementalBeamStrain::validParams();
  params.addClassDescription("Compute a rotation increment for finite rotations of the beam and "
                             "computes the small/large strain increments in the current rotated "
                             "configuration of the beam.");
  return params;
}

ComputeFiniteBeamStrain::ComputeFiniteBeamStrain(const InputParameters & parameters)
  : ComputeIncrementalBeamStrain(parameters),
    _total_rotation_old(getMaterialPropertyOld<RankTwoTensor>("total_rotation"))
{
}

void
ComputeFiniteBeamStrain::computeRotation()
{
  // First - convert the differential beam displacement to the beam configuration at previous time
  // step
  const RealVectorValue delta_disp_local(_total_rotation_old[0] * (_disp1 - _disp0));

  // Second - calculate the incremental rotation matrix using Euler angles
  const Real intermediate_length_1 =
      std::sqrt(Utility::pow<2>(_original_length[0] + delta_disp_local(0)) +
                Utility::pow<2>(delta_disp_local(2)));
  const Real cos_alpha = (_original_length[0] + delta_disp_local(0)) / intermediate_length_1;
  const Real sin_alpha = std::sqrt(1.0 - Utility::pow<2>(cos_alpha));

  const Real intermediate_length_2 =
      std::sqrt(Utility::pow<2>(intermediate_length_1) + Utility::pow<2>(delta_disp_local(1)));
  const Real sin_beta = delta_disp_local(1) / intermediate_length_2;
  const Real cos_beta = std::sqrt(1.0 - Utility::pow<2>(sin_beta));

  const RealVectorValue rotation_d_1(cos_alpha * cos_beta, sin_beta, sin_alpha * cos_beta);
  const RealVectorValue rotation_d_2(-cos_alpha * sin_beta, cos_beta, -sin_alpha * sin_beta);
  const RealVectorValue rotation_d_3(-sin_alpha, 0.0, cos_alpha);

  const auto rotation_d =
      RankTwoTensor ::initializeFromRows(rotation_d_1, rotation_d_2, rotation_d_3);

  // Convert average rotational displacement to the beam configuration at previous time step
  const RealVectorValue avg_rot_local(_total_rotation_old[0] * (_rot0 + _rot1));

  const Real gamma_increment =
      0.5 * (rotation_d_1(0) * avg_rot_local(0) + rotation_d_1(1) * avg_rot_local(1) +
             rotation_d_1(2) * avg_rot_local(2));

  RankTwoTensor rotation_a;
  rotation_a(0, 0) = 1.0;
  rotation_a(1, 1) = std::cos(gamma_increment);
  rotation_a(1, 2) = std::sin(gamma_increment);
  rotation_a(2, 1) = -rotation_a(1, 2);
  rotation_a(2, 2) = rotation_a(1, 1);

  // Total rotation matrix from global configuration to beam local config at current time
  _total_rotation[0] = rotation_a * rotation_d * _total_rotation_old[0];
}
