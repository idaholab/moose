/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeFiniteBeamStrain.h"
#include "Assembly.h"
#include "NonlinearSystem.h"
#include "MooseVariable.h"

#include "libmesh/quadrature.h"
#include "libmesh/utility.h"

registerMooseObject("TensorMechanicsApp", ComputeFiniteBeamStrain);

template <>
InputParameters
validParams<ComputeFiniteBeamStrain>()
{
  InputParameters params = validParams<ComputeIncrementalBeamStrain>();
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
ComputeFiniteBeamStrain::computeProperties()
{
  // Calculation of rotation matrix from original beam local configuration to configuration at time
  // t
  computeRotation();

  ComputeIncrementalBeamStrain::computeProperties();
}

void
ComputeFiniteBeamStrain::computeRotation()
{
  // First - calculate difference in beam displacement between node 1 and node 0 in global
  // coordinate system

  // fetch the two end nodes for _current_elem
  std::vector<Node *> node;
  for (unsigned int i = 0; i < 2; ++i)
    node.push_back(_current_elem->get_node(i));

  // compute original length of beam element
  RealGradient dxyz;
  for (unsigned int i = 0; i < _ndisp; ++i)
    dxyz(i) = (*node[1])(i) - (*node[0])(i);

  Real original_length = dxyz.norm();

  // Fetch the solution for the two end nodes at the current and previous time step
  NonlinearSystemBase & nonlinear_sys = _fe_problem.getNonlinearSystemBase();
  const NumericVector<Number> & sol = *nonlinear_sys.currentSolution();
  const NumericVector<Number> & sol_old = nonlinear_sys.solutionOld();

  RealVectorValue disp0(3, 0.0), disp1(3, 0.0);
  RealVectorValue rot0(3, 0.0), rot1(3, 0.0);
  RealVectorValue delta_disp(3, 0.0);
  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    disp0(i) = sol(node[0]->dof_number(nonlinear_sys.number(), _disp_num[i], 0)) -
               sol_old(node[0]->dof_number(nonlinear_sys.number(), _disp_num[i], 0));
    disp1(i) = sol(node[1]->dof_number(nonlinear_sys.number(), _disp_num[i], 0)) -
               sol_old(node[1]->dof_number(nonlinear_sys.number(), _disp_num[i], 0));
    rot0(i) = sol(node[0]->dof_number(nonlinear_sys.number(), _rot_num[i], 0)) -
              sol_old(node[0]->dof_number(nonlinear_sys.number(), _rot_num[i], 0));
    rot1(i) = sol(node[1]->dof_number(nonlinear_sys.number(), _rot_num[i], 0)) -
              sol_old(node[1]->dof_number(nonlinear_sys.number(), _rot_num[i], 0));

    delta_disp(i) = disp1(i) - disp0(i);
  }

  // Second - convert the differential beam displacement to the beam configuration at previous time
  // step
  RealVectorValue delta_disp_local(_total_rotation_old[0] * delta_disp);

  // Third - calculate the incremental rotation matrix using Euler angles
  Real intermediate_length_1 = std::sqrt(Utility::pow<2>(original_length + delta_disp_local(0)) +
                                         Utility::pow<2>(delta_disp_local(2)));
  Real cos_alpha = (original_length + delta_disp_local(0)) / intermediate_length_1;
  Real sin_alpha = std::sqrt(1.0 - Utility::pow<2>(cos_alpha));

  Real intermediate_length_2 =
      std::sqrt(Utility::pow<2>(intermediate_length_1) + Utility::pow<2>(delta_disp_local(1)));
  Real sin_beta = delta_disp_local(1) / intermediate_length_2;
  Real cos_beta = std::sqrt(1.0 - Utility::pow<2>(sin_beta));

  RealVectorValue rotation_d_1(cos_alpha * cos_beta, sin_beta, sin_alpha * cos_beta);
  RealVectorValue rotation_d_2(-cos_alpha * sin_beta, cos_beta, -sin_alpha * sin_beta);
  RealVectorValue rotation_d_3(-sin_alpha, 0.0, cos_alpha);

  RankTwoTensor rotation_d(rotation_d_1, rotation_d_2, rotation_d_3);

  // Convert average rotational displacement to the beam configuration at previous time step
  RealVectorValue avg_rot_local(_total_rotation_old[0] * (rot0 + rot1));

  Real gamma_increment =
      0.5 * (rotation_d_1(0) * avg_rot_local(0) + rotation_d_1(1) * avg_rot_local(1) +
             rotation_d_1(2) * avg_rot_local(2));

  RankTwoTensor rotation_a;
  rotation_a.zero();
  rotation_a(0, 0) = 1.0;
  rotation_a(1, 1) = std::cos(gamma_increment);
  rotation_a(1, 2) = std::sin(gamma_increment);
  rotation_a(2, 1) = -rotation_a(1, 2);
  rotation_a(2, 2) = rotation_a(1, 1);

  // Total rotation matrix from global configuration to beam local config at current time
  _total_rotation[0] = rotation_a * rotation_d * _total_rotation_old[0];
}
