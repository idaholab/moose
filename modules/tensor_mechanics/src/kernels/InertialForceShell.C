//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InertialForceShell.h"
#include "SubProblem.h"
#include "libmesh/utility.h"
#include "MooseVariable.h"
#include "Assembly.h"
#include "NonlinearSystem.h"
#include "AuxiliarySystem.h"
#include "MooseMesh.h"
#include "DenseMatrix.h"

// #include "Coupleable.h"

#include "libmesh/utility.h"
#include "libmesh/quadrature.h"
#include "libmesh/enum_quadrature_type.h"
#include "libmesh/fe_type.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature_gauss.h"

registerADMooseObject("TensorMechanicsApp", ADInertialForceShell);

defineADValidParams(
    ADInertialForceShell,
    ADTimeKernel,
    params.addClassDescription("Calculates the residual for the inertial force/moment and the "
                               "contribution of mass dependent Rayleigh damping and HHT time "
                               "integration scheme.");
    params.set<bool>("use_displaced_mesh") = true;
    params.addRequiredCoupledVar(
        "rotations",
        "The rotational variables appropriate for the simulation geometry and coordinate system");
    params.addRequiredCoupledVar(
        "displacements",
        "The displacement variables appropriate for the simulation geometry and coordinate system");
    params.addCoupledVar("velocities", "Translational velocity variables");
    params.addCoupledVar("accelerations", "Translational acceleration variables");
    params.addCoupledVar("rotational_velocities", "Rotational velocity variables");
    params.addCoupledVar("rotational_accelerations", "Rotational acceleration variables");
    params.addParam<MaterialPropertyName>(
        "density",
        "density",
        "Name of Material Property  or a constant real number defining the density of the beam.");
    params.addRequiredRangeCheckedParam<unsigned int>(
        "component",
        "component<5",
        "An integer corresponding to the direction "
        "the variable this kernel acts in. (0 for disp_x, "
        "1 for disp_y, 2 for disp_z, 3 for alpha, and 4 for beta)");
    params.addRequiredParam<Real>("thickness", "Generic thickness --to be upgraded");
    params.addParam<MaterialPropertyName>("eta",
                                          0.0,
                                          "Name of material property or a constant real "
                                          "number defining the eta parameter for the "
                                          "Rayleigh damping."););

template <ComputeStage compute_stage>
ADInertialForceShell<compute_stage>::ADInertialForceShell(const InputParameters & parameters)
  : ADTimeKernel<compute_stage>(parameters),
    _has_velocities(isParamValid("velocities")),
    _has_rot_velocities(isParamValid("rotational_velocities")),
    _has_accelerations(isParamValid("accelerations")),
    _has_rot_accelerations(isParamValid("rotational_accelerations")),
    _density(getMaterialProperty<Real>("density")),
    _nrot(coupledComponents("rotations")),
    _ndisp(coupledComponents("displacements")),
    _rot_num(_nrot),
    _disp_num(_ndisp),
    _vel_num(_ndisp),
    _accel_num(_ndisp),
    _rot_vel_num(_nrot),
    _rot_accel_num(_nrot),
    _component(getParam<unsigned int>("component")),
    _local_force(4),
    _local_moment(4),
    _nodes(4),
    _v1(4),
    _v2(4),
    _node_normal(4),
    _eta(getMaterialProperty<Real>("eta")),
    _transformation_matrix(getADMaterialProperty<RankTwoTensor>("rotation_t_points_0")),
    _J_map(getADMaterialProperty<Real>("J_mapping_t_points_0")),
    _thickness(getParam<Real>("thickness")),
    _isRotationMatrixComputed(false)
{
  // _thickness(getParam<Real>("thickness")),
  // Checking for consistency between the length of the provided rotations and displacements vector
  if (_ndisp != 3 || _nrot != 2)
    mooseError("InertialForceShell: The number of variables supplied in 'displacements' "
               "must be 3 and that in 'rotations' must be 2.");

  // fetch coupled displacements and rotations
  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    MooseVariable * disp_variable = getVar("displacements", i);
    _disp_num[i] = disp_variable->number();
  }
  for (unsigned int i = 0; i < _nrot; ++i)
  {
    MooseVariable * rot_variable = getVar("rotations", i);
    _rot_num[i] = rot_variable->number();
  }

  // Create sets of tangent vectors to each element node
  // in the initial configuration.

  _x2(1) = 1;
  _x3(2) = 1;

  Moose::out << " End of Constructor \n";
}

template <>
void
ADInertialForceShell<RESIDUAL>::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  precalculateResidual();

  // Rotation matrix from global to original shell local configuration
  // Grabbed at a point in the shell.

  if (_isRotationMatrixComputed == false)
  {
    _original_local_config = _transformation_matrix[0];
    _isRotationMatrixComputed = true;
  }

  if (_dt == 0)
    mooseError("Ever called with dt == 0?");

  _nodes[0] = _current_elem->node_ptr(0);
  _nodes[1] = _current_elem->node_ptr(1);
  _nodes[2] = _current_elem->node_ptr(2);
  _nodes[3] = _current_elem->node_ptr(3);

  RealVectorValue x = (*_nodes[1] - *_nodes[0]);
  RealVectorValue y = (*_nodes[3] - *_nodes[0]);
  RealVectorValue normal = x.cross(y);
  normal /= normal.norm();

  // Lines below cause a seg fault

  _node_normal[0] = normal;
  _node_normal[1] = normal;
  _node_normal[2] = normal;
  _node_normal[3] = normal;

  // compute nodal local axis
  for (unsigned int k = 0; k < _nodes.size(); ++k)
  {
    _v1[k] = _x2.cross(_node_normal[k]);
    _v1[k] /= _x2.norm() * _node_normal[k].norm();

    // If x2 is parallel to node normal, set V1 to x3
    if (MooseUtils::absoluteFuzzyEqual(_v1[k].norm(), 0.0, 1e-6))
      _v1[k] = _x3;

    _v2[k] = _node_normal[k].cross(_v1[k]);
  }

  /* ----------------------------------------------------- */

  if (_dt != 0.0)
  {
    // Fetch the solution for the two end nodes at time t
    NonlinearSystemBase & nonlinear_sys = _fe_problem.getNonlinearSystemBase();

    if (!nonlinear_sys.solutionUDot())
      mooseError("InertialForceShell: Time derivative of solution (`u_dot`) is not stored. Please "
                 "set uDotRequested() to true in FEProblemBase before requesting `u_dot`.");

    if (!nonlinear_sys.solutionUDotOld())
      mooseError("InertialForceShell: Old time derivative of solution (`u_dot_old`) is not "
                 "stored. Please set uDotOldRequested() to true in FEProblemBase before "
                 "requesting `u_dot_old`.");

    if (!nonlinear_sys.solutionUDotDot())
      mooseError("InertialForceShell: Second time derivative of solution (`u_dotdot`) is not "
                 "stored. Please set uDotDotRequested() to true in FEProblemBase before "
                 "requesting `u_dotdot`.");

    const NumericVector<Number> & vel = *nonlinear_sys.solutionUDot();
    //    const NumericVector<Number> & vel_old = *nonlinear_sys.solutionUDotOld();
    const NumericVector<Number> & accel = *nonlinear_sys.solutionUDotDot();

    for (unsigned int i = 0; i < _ndisp; ++i)
    {
      // translational velocities and accelerations
      unsigned int dof_index_0 = _nodes[0]->dof_number(nonlinear_sys.number(), _disp_num[i], 0);
      unsigned int dof_index_1 = _nodes[1]->dof_number(nonlinear_sys.number(), _disp_num[i], 0);
      unsigned int dof_index_2 = _nodes[2]->dof_number(nonlinear_sys.number(), _disp_num[i], 0);
      unsigned int dof_index_3 = _nodes[3]->dof_number(nonlinear_sys.number(), _disp_num[i], 0);

      _vel_0(i) = vel(dof_index_0);
      _vel_1(i) = vel(dof_index_1);
      _vel_2(i) = vel(dof_index_2);
      _vel_3(i) = vel(dof_index_3);

      _accel_0(i) = accel(dof_index_0);
      _accel_1(i) = accel(dof_index_1);
      _accel_2(i) = accel(dof_index_2);
      _accel_3(i) = accel(dof_index_3);
    }

    for (unsigned int i = 0; i < _nrot; ++i)
    {
      // rotational velocities and accelerations
      unsigned int dof_index_0 = _nodes[0]->dof_number(nonlinear_sys.number(), _rot_num[i], 0);
      unsigned int dof_index_1 = _nodes[1]->dof_number(nonlinear_sys.number(), _rot_num[i], 0);
      unsigned int dof_index_2 = _nodes[2]->dof_number(nonlinear_sys.number(), _rot_num[i], 0);
      unsigned int dof_index_3 = _nodes[3]->dof_number(nonlinear_sys.number(), _rot_num[i], 0);

      _rot_vel_0(i) = vel(dof_index_0);
      _rot_vel_1(i) = vel(dof_index_1);
      _rot_vel_2(i) = vel(dof_index_2);
      _rot_vel_3(i) = vel(dof_index_3);

      _rot_accel_0(i) = accel(dof_index_0);
      _rot_accel_1(i) = accel(dof_index_1);
      _rot_accel_2(i) = accel(dof_index_2);
      _rot_accel_3(i) = accel(dof_index_3);
    }
    // transform translational and rotational velocities and accelerations to the initial local
    // configuration of the beam
    _local_vel_0 = _original_local_config * _vel_0;
    _local_vel_1 = _original_local_config * _vel_1;
    _local_vel_2 = _original_local_config * _vel_2;
    _local_vel_3 = _original_local_config * _vel_3;

    _local_accel_0 = _original_local_config * _accel_0;
    _local_accel_1 = _original_local_config * _accel_1;
    _local_accel_2 = _original_local_config * _accel_2;
    _local_accel_3 = _original_local_config * _accel_3;

    _local_rot_vel_0 = _original_local_config * _rot_vel_0;
    _local_rot_vel_1 = _original_local_config * _rot_vel_1;
    _local_rot_vel_2 = _original_local_config * _rot_vel_2;
    _local_rot_vel_3 = _original_local_config * _rot_vel_3;

    _local_rot_accel_0 = _original_local_config * _rot_accel_0;
    _local_rot_accel_1 = _original_local_config * _rot_accel_1;
    _local_rot_accel_2 = _original_local_config * _rot_accel_2;
    _local_rot_accel_3 = _original_local_config * _rot_accel_3;
    // AMR

    // Conversions to DenseVector from RealVectorValue: Make a method out of this.
    DenseVector<Real> local_accel_dv_0(3);
    DenseVector<Real> local_accel_dv_1(3);
    DenseVector<Real> local_accel_dv_2(3);
    DenseVector<Real> local_accel_dv_3(3);

    DenseVector<Real> local_rot_accel_dv_0(3);
    DenseVector<Real> local_rot_accel_dv_1(3);
    DenseVector<Real> local_rot_accel_dv_2(3);
    DenseVector<Real> local_rot_accel_dv_3(3);

    DenseVector<Real> local_vel_dv_0(3);
    DenseVector<Real> local_vel_dv_1(3);
    DenseVector<Real> local_vel_dv_2(3);
    DenseVector<Real> local_vel_dv_3(3);

    DenseVector<Real> local_rot_vel_dv_0(3);
    DenseVector<Real> local_rot_vel_dv_1(3);
    DenseVector<Real> local_rot_vel_dv_2(3);
    DenseVector<Real> local_rot_vel_dv_3(3);

    //      Moose::out << "End of defining DenseVectors. \n";
    //      Moose::out << "_local_accel_0: " << _local_accel_0(2) << "\n";
    //      Moose::out << "_local_accel_1: " << _local_accel_1(2) << "\n";
    //      Moose::out << "_local_accel_2: " << _local_accel_2(2) << "\n";
    //      Moose::out << "_local_accel_3: " << _local_accel_3(2) << "\n";

    for (unsigned int i = 0; i < 3; i++)
    {
      local_accel_dv_0(i) = _local_accel_0(i);
      local_accel_dv_1(i) = _local_accel_1(i);
      local_accel_dv_2(i) = _local_accel_2(i);
      local_accel_dv_3(i) = _local_accel_3(i);

      local_rot_accel_dv_0(i) = _local_rot_accel_0(i);
      local_rot_accel_dv_1(i) = _local_rot_accel_1(i);
      local_rot_accel_dv_2(i) = _local_rot_accel_2(i);
      local_rot_accel_dv_3(i) = _local_rot_accel_3(i);

      local_vel_dv_0(i) = _local_vel_0(i);
      local_vel_dv_1(i) = _local_vel_1(i);
      local_vel_dv_2(i) = _local_vel_2(i);
      local_vel_dv_3(i) = _local_vel_3(i);

      local_rot_vel_dv_0(i) = _local_rot_vel_0(i);
      local_rot_vel_dv_1(i) = _local_rot_vel_1(i);
      local_rot_vel_dv_2(i) = _local_rot_vel_2(i);
      local_rot_vel_dv_3(i) = _local_rot_vel_3(i);
    }
    unsigned int dim = _current_elem->dim();

    // Update 0g vectors at plane quadrature points.
    _0g1_vector.clear();
    _0g1_vector.push_back(-0.5 * _thickness * _v1[0]);
    _0g1_vector.push_back(0.5 * _thickness * _v2[0]);

    DenseMatrix<Real> G1(3, 2);
    G1(0, 0) = _0g1_vector[0](0);
    G1(1, 0) = _0g1_vector[0](1);
    G1(2, 0) = _0g1_vector[0](2);
    G1(0, 1) = _0g1_vector[1](0);
    G1(1, 1) = _0g1_vector[1](1);
    G1(2, 1) = _0g1_vector[1](2);
    DenseMatrix<Real> G1T(2, 3);
    G1.get_transpose(G1T);

    _0g2_vector.clear();
    _0g2_vector.push_back(-0.5 * _thickness * _v1[1]);
    _0g2_vector.push_back(0.5 * _thickness * _v2[1]);

    DenseMatrix<Real> G2(3, 2);
    G2(0, 0) = _0g2_vector[0](0);
    G2(1, 0) = _0g2_vector[0](1);
    G2(2, 0) = _0g2_vector[0](2);
    G2(0, 1) = _0g2_vector[1](0);
    G2(1, 1) = _0g2_vector[1](1);
    G2(2, 1) = _0g2_vector[1](2);

    DenseMatrix<Real> G2T(2, 3);
    G2.get_transpose(G2T);

    _0g3_vector.clear();
    _0g3_vector.push_back(-0.5 * _thickness * _v1[2]);
    _0g3_vector.push_back(0.5 * _thickness * _v2[2]);

    DenseMatrix<Real> G3(3, 2);
    G3(0, 0) = _0g3_vector[0](0);
    G3(1, 0) = _0g3_vector[0](1);
    G3(2, 0) = _0g3_vector[0](2);
    G3(0, 1) = _0g3_vector[1](0);
    G3(1, 1) = _0g3_vector[1](1);
    G3(2, 1) = _0g3_vector[1](2);

    DenseMatrix<Real> G3T(2, 3);
    G3.get_transpose(G3T);

    _0g4_vector.clear();
    _0g4_vector.push_back(-0.5 * _thickness * _v1[3]);
    _0g4_vector.push_back(0.5 * _thickness * _v2[3]);

    DenseMatrix<Real> G4(3, 2);
    G4(0, 0) = _0g4_vector[0](0);
    G4(1, 0) = _0g4_vector[0](1);
    G4(2, 0) = _0g4_vector[0](2);
    G4(0, 1) = _0g4_vector[1](0);
    G4(1, 1) = _0g4_vector[1](1);
    G4(2, 1) = _0g4_vector[1](2);

    DenseMatrix<Real> G4T(2, 3);
    G4.get_transpose(G4T);

    //            Moose::out << "End of defining local_rot_accel_dv. \n";
    //            Moose::out << "local_rot_accel_dv_0: " << local_rot_accel_dv_0(0) << "\n";
    //            Moose::out << "local_rot_accel_dv_1: " << local_rot_accel_dv_1(0) << "\n";
    //            Moose::out << "local_rot_accel_dv_2: " << local_rot_accel_dv_2(0) << "\n";
    //            Moose::out << "local_rot_accel_dv_3: " << local_rot_accel_dv_3(0) << "\n";
    //
    //            Moose::out << "End of defining local_rot_accel_dv. XXXXXXXXXXX \n";
    //            Moose::out << "local_accel_dv_0: " << local_accel_dv_0(2) << "\n";
    //            Moose::out << "local_accel_dv_1: " << local_accel_dv_1(2) << "\n";
    //            Moose::out << "local_accel_dv_2: " << local_accel_dv_2(2) << "\n";
    //            Moose::out << "local_accel_dv_3: " << local_accel_dv_3(2) << "\n";

    std::vector<DenseVector<Real>> local_acc;
    local_acc.resize(4);
    local_acc[0].resize(3);
    local_acc[1].resize(3);
    local_acc[2].resize(3);
    local_acc[3].resize(3);

    local_acc[0] = local_accel_dv_0;
    local_acc[1] = local_accel_dv_1;
    local_acc[2] = local_accel_dv_2;
    local_acc[3] = local_accel_dv_3;

    std::vector<DenseVector<Real>> local_rot_acc;
    local_rot_acc.resize(4);
    local_rot_acc[0].resize(3);
    local_rot_acc[1].resize(3);
    local_rot_acc[2].resize(3);
    local_rot_acc[3].resize(3);
    local_rot_acc[0] = local_rot_accel_dv_0;
    local_rot_acc[1] = local_rot_accel_dv_1;
    local_rot_acc[2] = local_rot_accel_dv_2;
    local_rot_acc[3] = local_rot_accel_dv_3;

    // Velocity for Rayleigh damping
    std::vector<DenseVector<Real>> local_vel;
    local_vel.resize(4);
    local_vel[0].resize(3);
    local_vel[1].resize(3);
    local_vel[2].resize(3);
    local_vel[3].resize(3);

    local_vel[0] = local_vel_dv_0;
    local_vel[1] = local_vel_dv_1;
    local_vel[2] = local_vel_dv_2;
    local_vel[3] = local_vel_dv_3;

    std::vector<DenseVector<Real>> local_rot_vel;
    local_rot_vel.resize(4);
    local_rot_vel[0].resize(3);
    local_rot_vel[1].resize(3);
    local_rot_vel[2].resize(3);
    local_rot_vel[3].resize(3);

    local_rot_vel[0] = local_rot_vel_dv_0;
    local_rot_vel[1] = local_rot_vel_dv_1;
    local_rot_vel[2] = local_rot_vel_dv_2;
    local_rot_vel[3] = local_rot_vel_dv_3;

    FEType fe_type(Utility::string_to_enum<Order>("First"),
                   Utility::string_to_enum<FEFamily>("LAGRANGE"));
    auto & fe = _fe_problem.assembly(_tid).getFE(fe_type, dim);
    _dphidxi_map = fe->get_fe_map().get_dphidxi_map();
    _dphideta_map = fe->get_fe_map().get_dphideta_map();
    _phi_map = fe->get_fe_map().get_phi_map();

    // quadrature points in isoparametric space
    _2d_points = _qrule->get_points(); // would be in 2D

    std::vector<const Node *> nodes;
    for (unsigned int i = 0; i < 4; ++i)
      nodes.push_back(_current_elem->node_ptr(i));

    // Loosely following notation in: "On finite element nonlinear analysis of general shell
    // structures", PhD thesis by Said Bolourchi (1975).
    for (unsigned int i = 0; i < _ndisp; i++)
      for (unsigned int j = 0; j < 4; j++)
        _local_force[j](i) = 0.0;

    for (unsigned int i = 0; i < _nrot; i++)
      for (unsigned int j = 0; j < 4; j++)
        _local_moment[j](i) = 0.0;

    for (unsigned int qp_xy = 0; qp_xy < _2d_points.size(); ++qp_xy)
    {

      Real factor_qxy = _ad_coord[qp_xy] * _ad_JxW[qp_xy];
      // Real factor_qxy = _2d_weights[qp_xy] * _J_map[qp_xy];

      for (unsigned int dim = 0; dim < 3; dim++)
      {
        _local_force[0](dim) +=
            factor_qxy * (_phi_map[0][qp_xy] * _phi_map[0][qp_xy] * local_acc[0](dim) +
                          _phi_map[0][qp_xy] * _phi_map[1][qp_xy] * local_acc[1](dim) +
                          _phi_map[0][qp_xy] * _phi_map[2][qp_xy] * local_acc[2](dim) +
                          _phi_map[0][qp_xy] * _phi_map[3][qp_xy] * local_acc[3](dim));

        if (_eta[0] > TOLERANCE * TOLERANCE)
          _local_force[0](dim) += factor_qxy * _eta[0] *
                                  (_phi_map[0][qp_xy] * _phi_map[0][qp_xy] * local_vel[0](dim) +
                                   _phi_map[0][qp_xy] * _phi_map[1][qp_xy] * local_vel[1](dim) +
                                   _phi_map[0][qp_xy] * _phi_map[2][qp_xy] * local_vel[2](dim) +
                                   _phi_map[0][qp_xy] * _phi_map[3][qp_xy] * local_vel[3](dim));

        _local_force[1](dim) +=
            factor_qxy * (_phi_map[1][qp_xy] * _phi_map[0][qp_xy] * local_acc[0](dim) +
                          _phi_map[1][qp_xy] * _phi_map[1][qp_xy] * local_acc[1](dim) +
                          _phi_map[1][qp_xy] * _phi_map[2][qp_xy] * local_acc[2](dim) +
                          _phi_map[1][qp_xy] * _phi_map[3][qp_xy] * local_acc[3](dim));

        if (_eta[0] > TOLERANCE * TOLERANCE)
          _local_force[1](dim) += factor_qxy * _eta[0] *
                                  (_phi_map[1][qp_xy] * _phi_map[0][qp_xy] * local_vel[0](dim) +
                                   _phi_map[1][qp_xy] * _phi_map[1][qp_xy] * local_vel[1](dim) +
                                   _phi_map[1][qp_xy] * _phi_map[2][qp_xy] * local_vel[2](dim) +
                                   _phi_map[1][qp_xy] * _phi_map[3][qp_xy] * local_vel[3](dim));
        _local_force[2](dim) +=
            factor_qxy * (_phi_map[2][qp_xy] * _phi_map[0][qp_xy] * local_acc[0](dim) +
                          _phi_map[2][qp_xy] * _phi_map[1][qp_xy] * local_acc[1](dim) +
                          _phi_map[2][qp_xy] * _phi_map[2][qp_xy] * local_acc[2](dim) +
                          _phi_map[2][qp_xy] * _phi_map[3][qp_xy] * local_acc[3](dim));

        if (_eta[0] > TOLERANCE * TOLERANCE)
          _local_force[2](dim) += factor_qxy * _eta[0] *
                                  (_phi_map[2][qp_xy] * _phi_map[0][qp_xy] * local_vel[0](dim) +
                                   _phi_map[2][qp_xy] * _phi_map[1][qp_xy] * local_vel[1](dim) +
                                   _phi_map[2][qp_xy] * _phi_map[2][qp_xy] * local_vel[2](dim) +
                                   _phi_map[2][qp_xy] * _phi_map[3][qp_xy] * local_vel[3](dim));

        _local_force[3](dim) +=
            factor_qxy * (_phi_map[3][qp_xy] * _phi_map[0][qp_xy] * local_acc[0](dim) +
                          _phi_map[3][qp_xy] * _phi_map[1][qp_xy] * local_acc[1](dim) +
                          _phi_map[3][qp_xy] * _phi_map[2][qp_xy] * local_acc[2](dim) +
                          _phi_map[3][qp_xy] * _phi_map[3][qp_xy] * local_acc[3](dim));

        if (_eta[0] > TOLERANCE * TOLERANCE)
          _local_force[3](dim) +=
              factor_qxy * (_phi_map[3][qp_xy] * _phi_map[0][qp_xy] * local_vel[0](dim) +
                            _phi_map[3][qp_xy] * _phi_map[1][qp_xy] * local_vel[1](dim) +
                            _phi_map[3][qp_xy] * _phi_map[2][qp_xy] * local_vel[2](dim) +
                            _phi_map[3][qp_xy] * _phi_map[3][qp_xy] * local_vel[3](dim));
      }
      // if (_eta[0] > TOLERANCE*TOLERANCE) addRayleighDampingLocalForce(local_vel, local_rot_vel);
      // Should group all Rayleigh contributions together for performance and readability.
      Real rot_thickness = _thickness * _thickness * _thickness / 48.0;

      DenseVector<Real> momentInertia(3);
      momentInertia(0) = (G1(0, 0) * local_rot_acc[0](0) + G1(0, 1) * local_rot_acc[0](1) +
                          G2(0, 0) * local_rot_acc[1](0) + G2(0, 1) * local_rot_acc[1](1) +
                          G3(0, 0) * local_rot_acc[2](0) + G3(0, 1) * local_rot_acc[2](1) +
                          G4(0, 0) * local_rot_acc[3](0) + G4(0, 1) * local_rot_acc[3](1));

      momentInertia(1) = (G1(1, 0) * local_rot_acc[0](0) + G1(1, 1) * local_rot_acc[0](1) +
                          G2(1, 0) * local_rot_acc[1](0) + G2(1, 1) * local_rot_acc[1](1) +
                          G3(1, 0) * local_rot_acc[2](0) + G3(1, 1) * local_rot_acc[2](1) +
                          G4(1, 0) * local_rot_acc[3](0) + G4(1, 1) * local_rot_acc[3](1));

      momentInertia(2) = (G1(2, 0) * local_rot_acc[0](0) + G1(2, 1) * local_rot_acc[0](1) +
                          G2(2, 0) * local_rot_acc[1](0) + G2(2, 1) * local_rot_acc[1](1) +
                          G3(2, 0) * local_rot_acc[2](0) + G3(2, 1) * local_rot_acc[2](1) +
                          G4(2, 0) * local_rot_acc[3](0) + G4(2, 1) * local_rot_acc[3](1));

      _local_moment[0](0) += factor_qxy * rot_thickness *
                             (G1T(0, 0) * momentInertia(0) + G1T(0, 1) * momentInertia(1) +
                              G1T(0, 2) * momentInertia(2));

      _local_moment[0](1) += factor_qxy * rot_thickness *
                             (G1T(1, 0) * momentInertia(0) + G1T(1, 1) * momentInertia(1) +
                              G1T(1, 2) * momentInertia(2));

      _local_moment[1](0) += factor_qxy * rot_thickness *
                             (G1T(0, 0) * momentInertia(0) + G1T(0, 1) * momentInertia(1) +
                              G2T(0, 2) * momentInertia(2));

      _local_moment[1](1) += factor_qxy * rot_thickness *
                             (G1T(1, 0) * momentInertia(0) + G1T(1, 1) * momentInertia(1) +
                              G2T(1, 2) * momentInertia(2));

      _local_moment[2](0) += factor_qxy * rot_thickness *
                             (G1T(0, 0) * momentInertia(0) + G1T(0, 1) * momentInertia(1) +
                              G3T(0, 2) * momentInertia(2));

      _local_moment[2](1) += factor_qxy * rot_thickness *
                             (G1T(1, 0) * momentInertia(0) + G1T(1, 1) * momentInertia(1) +
                              G3T(1, 2) * momentInertia(2));

      _local_moment[3](0) += factor_qxy * rot_thickness *
                             (G1T(0, 0) * momentInertia(0) + G1T(0, 1) * momentInertia(1) +
                              G4T(0, 2) * momentInertia(2));

      _local_moment[3](1) += factor_qxy * rot_thickness *
                             (G1T(1, 0) * momentInertia(0) + G1T(1, 1) * momentInertia(1) +
                              G4T(1, 2) * momentInertia(2));
    }

    if (_component < 3)
    {
      _global_force_0 = _thickness * _local_force[0];
      _global_force_1 = _thickness * _local_force[1];
      _global_force_2 = _thickness * _local_force[2];
      _global_force_3 = _thickness * _local_force[3];

      _local_re(0) = _global_force_0(_component);
      _local_re(1) = _global_force_1(_component);
      _local_re(2) = _global_force_2(_component);
      _local_re(3) = _global_force_3(_component);
    }
    else
    {
      _global_moment_0 = _local_moment[0];
      _global_moment_1 = _local_moment[1];
      _global_moment_2 = _local_moment[2];
      _global_moment_3 = _local_moment[3];

      _local_re(0) = _global_moment_0(_component - 3);
      _local_re(1) = _global_moment_1(_component - 3);
      _local_re(2) = _global_moment_2(_component - 3);
      _local_re(3) = _global_moment_3(_component - 3);
    }

  }

  /* ----------------------------------------------------- */

  // Hardcoding meaningless values
  // Include here the Newmar-beta scheme

  accumulateTaggedLocalResidual();

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i = 0; i < _save_in.size(); i++)
      _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }
}

template <ComputeStage compute_stage>
void
ADInertialForceShell<compute_stage>::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());

  size_t ad_offset = _var.number() * _sys.getMaxVarNDofsPerElem();

  precalculateResidual();

  // Debug and find out of this means
  // Moose::out << "COMPUTE JACOBIAN STUFF  \n";

  if (_use_displaced_mesh)
    for (_i = 0; _i < _test.size(); _i++)
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      {
        for (_j = 0; _j < _var.phiSize(); _j++)
        {
        }
      }

  if (_use_displaced_mesh)
    for (_i = 0; _i < _test.size(); _i++)
    {
      // Moose::out << "_i value is: " << _i << "\n"; Each of the four nodes.
      DualReal residual = 0.0;
      for (_j = 0; _j < _var.phiSize(); _j++)
        _local_ke(_i, _j) += residual.derivatives()[ad_offset + _j];
    }
  else
    mooseError("Calculations of shell element inertia assumes the use of a displaced mesh");

  accumulateTaggedLocalMatrix();

  if (_has_diag_save_in && !_sys.computingScalingJacobian())
  {
    unsigned int rows = _local_ke.m();
    DenseVector<Number> diag(rows);
    for (unsigned int i = 0; i < rows; i++)
      diag(i) = _local_ke(i, i);

    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i = 0; i < _diag_save_in.size(); i++)
      _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
  }
}

template <ComputeStage compute_stage>
void
ADInertialForceShell<compute_stage>::computeADOffDiagJacobian()
{
  if (_residuals.size() != _test.size())
    _residuals.resize(_test.size(), 0);

  for (auto & r : _residuals)
    r = 0;

  precalculateResidual();
  if (_use_displaced_mesh)
  {
    _r = _ad_JxW[_qp];
    _r *= _ad_coord[_qp];
    for (_i = 0; _i < _test.size(); _i++)
      _residuals[_i] += 0.0;
  }
  else
    for (_i = 0; _i < _test.size(); _i++)
      _residuals[_i] += 0.0;

  auto & ce = _assembly.couplingEntries();
  for (const auto & it : ce)
  {
    MooseVariableFEBase & ivariable = *(it.first);
    MooseVariableFEBase & jvariable = *(it.second);

    unsigned int ivar = ivariable.number();
    unsigned int jvar = jvariable.number();

    if (ivar != _var.number())
      continue;

    size_t ad_offset = jvar * _sys.getMaxVarNDofsPerElem();

    prepareMatrixTag(_assembly, ivar, jvar);

    if (_local_ke.m() != _test.size() || _local_ke.n() != jvariable.phiSize())
      continue;

    precalculateResidual();
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < jvariable.phiSize(); _j++)
        _local_ke(_i, _j) += _residuals[_i].derivatives()[ad_offset + _j];

    accumulateTaggedLocalMatrix();
  }
}

template <>
void
ADInertialForceShell<JACOBIAN>::computeResidual()
{
}

template <>
void
ADInertialForceShell<RESIDUAL>::computeJacobian()
{
}

adBaseClass(ADInertialForceShell);
