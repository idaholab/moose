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
    params.addRangeCheckedParam<Real>("beta",
                                      "beta>0.0",
                                      "beta parameter for Newmark Time integration");
    params.addRangeCheckedParam<Real>("gamma",
                                      "gamma>0.0",
                                      "gamma parameter for Newmark Time integration");
    params.addParam<MaterialPropertyName>("eta",
                                          0.0,
                                          "Name of material property or a constant real "
                                          "number defining the eta parameter for the "
                                          "Rayleigh damping.");
    params.addRangeCheckedParam<Real>(
        "alpha",
        0.0,
        "alpha >= -0.3333 & alpha <= 0.0",
        "Alpha parameter for mass dependent numerical damping induced "
        "by HHT time integration scheme");
    params.addParam<MaterialPropertyName>(
        "density",
        "density",
        "Name of Material Property  or a constant real number defining the density of the beam.");
    params.addRequiredRangeCheckedParam<unsigned int>(
        "component",
        "component<6",
        "An integer corresponding to the direction "
        "the variable this kernel acts in. (0 for disp_x, "
        "1 for disp_y, 2 for disp_z, 3 for rot_x, 4 for rot_y and 5 for rot_z)"););

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
    _eta(getMaterialProperty<Real>("eta")),
    _alpha(getParam<Real>("alpha")),
    _original_local_config(getMaterialPropertyByName<RankTwoTensor>("initial_rotation")),
    _original_length(getMaterialPropertyByName<Real>("original_length")),
    _component(getParam<unsigned int>("component")),
    _local_force(4),
    _local_moment(4),
    _v1(4),
    _v2(4),
    _thickness(coupledValue("thickness"))
{
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

  for (unsigned int i = 0; i < 4; ++i)
    _nodes[i] = _current_elem->node_ptr(i);

  RealVectorValue x = (*_nodes[1] - *_nodes[0]);
  RealVectorValue y = (*_nodes[3] - *_nodes[0]);
  RealVectorValue normal = x.cross(y);
  normal /= normal.norm();

  for (unsigned int k = 0; k < 4; ++k)
    _node_normal[k] = normal;

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
}

template <ComputeStage compute_stage>
ADReal
ADInertialForceShell<compute_stage>::computeQpResidual()
{
  prepareVectorTag(_assembly, _var.number());

  if (_dt != 0.0)
  {
    // fetch the four nodes for _current_elem
    std::vector<const Node *> node;
    for (unsigned int i = 0; i < 4; ++i)
      node.push_back(_current_elem->node_ptr(i));

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
    const NumericVector<Number> & vel_old = *nonlinear_sys.solutionUDotOld();
    const NumericVector<Number> & accel = *nonlinear_sys.solutionUDotDot();

    for (unsigned int i = 0; i < _ndisp; ++i)
    {
      // translational velocities and accelerations
      unsigned int dof_index_0 = node[0]->dof_number(nonlinear_sys.number(), _disp_num[i], 0);
      unsigned int dof_index_1 = node[1]->dof_number(nonlinear_sys.number(), _disp_num[i], 0);
      unsigned int dof_index_2 = node[2]->dof_number(nonlinear_sys.number(), _disp_num[i], 0);
      unsigned int dof_index_3 = node[3]->dof_number(nonlinear_sys.number(), _disp_num[i], 0);

      _vel_0(i) = vel(dof_index_0);
      _vel_1(i) = vel(dof_index_1);
      _vel_2(i) = vel(dof_index_2);
      _vel_3(i) = vel(dof_index_3);

      _vel_old_0(i) = vel_old(dof_index_0);
      _vel_old_1(i) = vel_old(dof_index_1);
      _vel_old_2(i) = vel_old(dof_index_2);
      _vel_old_3(i) = vel_old(dof_index_3);

      _accel_0(i) = accel(dof_index_0);
      _accel_1(i) = accel(dof_index_1);
      _accel_2(i) = accel(dof_index_2);
      _accel_3(i) = accel(dof_index_3);
    }

    for (unsigned int i = 0; i < _nrot; ++i)
    {
      // rotational velocities and accelerations
      unsigned int dof_index_0 = node[0]->dof_number(nonlinear_sys.number(), _rot_num[i], 0);
      unsigned int dof_index_1 = node[1]->dof_number(nonlinear_sys.number(), _rot_num[i], 0);
      unsigned int dof_index_2 = node[2]->dof_number(nonlinear_sys.number(), _rot_num[i], 0);
      unsigned int dof_index_3 = node[3]->dof_number(nonlinear_sys.number(), _rot_num[i], 0);

      _rot_vel_0(i) = vel(dof_index_0);
      _rot_vel_1(i) = vel(dof_index_1);
      _rot_vel_2(i) = vel(dof_index_2);
      _rot_vel_3(i) = vel(dof_index_3);

      _rot_vel_old_0(i) = vel_old(dof_index_0);
      _rot_vel_old_1(i) = vel_old(dof_index_1);
      _rot_vel_old_2(i) = vel_old(dof_index_2);
      _rot_vel_old_3(i) = vel_old(dof_index_3);

      _rot_accel_0(i) = accel(dof_index_0);
      _rot_accel_1(i) = accel(dof_index_1);
      _rot_accel_2(i) = accel(dof_index_2);
      _rot_accel_3(i) = accel(dof_index_3);
    }

    // transform translational and rotational velocities and accelerations to the initial local
    // configuration of the beam
    _local_vel_old_0 = _original_local_config[0] * _vel_old_0;
    _local_vel_old_1 = _original_local_config[0] * _vel_old_1;
    _local_vel_old_2 = _original_local_config[0] * _vel_old_2;
    _local_vel_old_3 = _original_local_config[0] * _vel_old_3;

    _local_vel_0 = _original_local_config[0] * _vel_0;
    _local_vel_1 = _original_local_config[0] * _vel_1;
    _local_vel_2 = _original_local_config[0] * _vel_2;
    _local_vel_3 = _original_local_config[0] * _vel_3;

    _local_accel_0 = _original_local_config[0] * _accel_0;
    _local_accel_1 = _original_local_config[0] * _accel_1;
    _local_accel_2 = _original_local_config[0] * _accel_2;
    _local_accel_3 = _original_local_config[0] * _accel_3;

    _local_rot_vel_old_0 = _original_local_config[0] * _rot_vel_old_0;
    _local_rot_vel_old_1 = _original_local_config[0] * _rot_vel_old_1;
    _local_rot_vel_old_2 = _original_local_config[0] * _rot_vel_old_2;
    _local_rot_vel_old_3 = _original_local_config[0] * _rot_vel_old_3;

    _local_rot_vel_0 = _original_local_config[0] * _rot_vel_0;
    _local_rot_vel_1 = _original_local_config[0] * _rot_vel_1;
    _local_rot_vel_2 = _original_local_config[0] * _rot_vel_2;
    _local_rot_vel_3 = _original_local_config[0] * _rot_vel_3;

    _local_rot_accel_0 = _original_local_config[0] * _rot_accel_0;
    _local_rot_accel_1 = _original_local_config[0] * _rot_accel_1;
    _local_rot_accel_2 = _original_local_config[0] * _rot_accel_2;
    _local_rot_accel_3 = _original_local_config[0] * _rot_accel_3;

    // AMR

    unsigned int dim = _current_elem->dim();

    FEType fe_type(Utility::string_to_enum<Order>("First"),
                   Utility::string_to_enum<FEFamily>("LAGRANGE"));
    auto & fe = _fe_problem.assembly(_tid).getFE(fe_type, dim);
    _dphidxi_map = fe->get_fe_map().get_dphidxi_map();
    _dphideta_map = fe->get_fe_map().get_dphideta_map();
    _phi_map = fe->get_fe_map().get_phi_map();

    _t_qrule = libmesh_make_unique<QGauss>(
        1, Utility::string_to_enum<Order>(getParam<std::string>("through_thickness_order")));
    _t_points = _t_qrule->get_points();
    _t_weights = _t_qrule->get_weights();

    // quadrature points in isoparametric space
    _2d_points = _qrule->get_points(); // would be in 2D
    _2d_weights = _qrule->get_weights();

    for (unsigned int i = 0; i < 4; ++i)
      _nodes[i] = _current_elem->node_ptr(i);

    // Loosely following notation in: "On finite element nonlinear analysis of general shell
    // structures", PhD thesis by Said Bolourchi (1975).

    for (unsigned int qp_xy = 0; qp_xy < _2d_points.size(); ++qp_xy)
    {
      // Update 0g vectors at plane quadrature points.
      _0g1_vector.clear();
      _0g1_vector.push_back(-0.5 * _thickness[qp_xy] * _v1[0]);
      _0g1_vector.push_back(0.5 * _thickness[qp_xy] * _v2[0]);

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
      _0g2_vector.push_back(-0.5 * _thickness[qp_xy] * _v1[1]);
      _0g2_vector.push_back(0.5 * _thickness[qp_xy] * _v2[1]);

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
      _0g3_vector.push_back(-0.5 * _thickness[qp_xy] * _v1[2]);
      _0g3_vector.push_back(0.5 * _thickness[qp_xy] * _v2[2]);

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
      _0g4_vector.push_back(-0.5 * _thickness[qp_xy] * _v1[3]);
      _0g4_vector.push_back(0.5 * _thickness[qp_xy] * _v2[3]);

      DenseMatrix<Real> G4(3, 2);
      G4(0, 0) = _0g4_vector[0](0);
      G4(1, 0) = _0g4_vector[0](1);
      G4(2, 0) = _0g4_vector[0](2);
      G4(0, 1) = _0g4_vector[1](0);
      G4(1, 1) = _0g4_vector[1](1);
      G4(2, 1) = _0g4_vector[1](2);

      DenseMatrix<Real> G4T(2, 3);
      G4.get_transpose(G4T);

      // Conversions to DenseVector from RealVectorValue: Make a method out of this.
      DenseVector<Real> local_accel_dv_0;
      DenseVector<Real> local_accel_dv_1;
      DenseVector<Real> local_accel_dv_2;
      DenseVector<Real> local_accel_dv_3;

      DenseVector<Real> local_rot_accel_dv_0;
      DenseVector<Real> local_rot_accel_dv_1;
      DenseVector<Real> local_rot_accel_dv_2;
      DenseVector<Real> local_rot_accel_dv_3;

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
      }

      std::vector<DenseVector<Real>> local_acc;
      local_acc.push_back(local_accel_dv_0);
      local_acc.push_back(local_accel_dv_1);
      local_acc.push_back(local_accel_dv_2);
      local_acc.push_back(local_accel_dv_3);

      std::vector<DenseVector<Real>> local_rot_acc;
      local_rot_acc.push_back(local_rot_accel_dv_0);
      local_rot_acc.push_back(local_rot_accel_dv_1);
      local_rot_acc.push_back(local_rot_accel_dv_2);
      local_rot_acc.push_back(local_rot_accel_dv_3);

      for (unsigned int qpz = 0; qpz < _t_points.size(); ++qpz)
      {
        // _local_force for each of the nodes. Try vector form first (containing three displacement
        // components)
        _2d_weights[qp_xy] * _t_weights[qpz] * _JxW[qp_xy] * _coord[qp_xy];

        for (unsigned int dim = 0; dim < 3; dim++)
        {
          _local_force[0](dim) +=
              _phi_map[0][qp_xy] * _phi_map[0][qp_xy] * local_acc[0](dim) +
              _phi_map[0][qp_xy] *
                  (G1(dim, 0) * local_rot_acc[0](0) + G1(dim, 1) * local_rot_acc[0](1)) +
              _phi_map[0][qp_xy] * _phi_map[1][qp_xy] * local_acc[1](dim) +
              _phi_map[0][qp_xy] *
                  (G2(dim, 0) * local_rot_acc[1](0) + G2(dim, 1) * local_rot_acc[1](1)) +
              _phi_map[0][qp_xy] * _phi_map[2][qp_xy] * local_acc[2](dim) +
              _phi_map[0][qp_xy] *
                  (G3(dim, 0) * local_rot_acc[2](0) + G3(dim, 1) * local_rot_acc[2](1)) +
              _phi_map[0][qp_xy] * _phi_map[3][qp_xy] * local_acc[3](dim) +
              _phi_map[0][qp_xy] *
                  (G4(dim, 0) * local_rot_acc[3](0) + G4(dim, 1) * local_rot_acc[3](1));

          _local_force[1](dim) +=
              _phi_map[1][qp_xy] * _phi_map[0][qp_xy] * local_acc[0](dim) +
              _phi_map[1][qp_xy] *
                  (G1(dim, 0) * local_rot_acc[0](0) + G1(dim, 1) * local_rot_acc[0](1)) +
              _phi_map[1][qp_xy] * _phi_map[1][qp_xy] * local_acc[1](dim) +
              _phi_map[1][qp_xy] *
                  (G2(dim, 0) * local_rot_acc[1](0) + G2(dim, 1) * local_rot_acc[1](1)) +
              _phi_map[1][qp_xy] * _phi_map[2][qp_xy] * local_acc[2](dim) +
              _phi_map[1][qp_xy] *
                  (G3(dim, 0) * local_rot_acc[2](0) + G3(dim, 1) * local_rot_acc[2](1)) +
              _phi_map[1][qp_xy] * _phi_map[3][qp_xy] * local_acc[3](dim) +
              _phi_map[1][qp_xy] *
                  (G4(dim, 0) * local_rot_acc[3](0) + G4(dim, 1) * local_rot_acc[3](1));

          _local_force[2](dim) +=
              _phi_map[2][qp_xy] * _phi_map[0][qp_xy] * local_acc[0](dim) +
              _phi_map[2][qp_xy] *
                  (G1(dim, 0) * local_rot_acc[0](0) + G1(dim, 1) * local_rot_acc[0](1)) +
              _phi_map[2][qp_xy] * _phi_map[1][qp_xy] * local_acc[1](dim) +
              _phi_map[2][qp_xy] *
                  (G2(dim, 0) * local_rot_acc[1](0) + G2(dim, 1) * local_rot_acc[1](1)) +
              _phi_map[2][qp_xy] * _phi_map[2][qp_xy] * local_acc[2](dim) +
              _phi_map[2][qp_xy] *
                  (G3(dim, 0) * local_rot_acc[2](0) + G3(dim, 1) * local_rot_acc[2](1)) +
              _phi_map[2][qp_xy] * _phi_map[3][qp_xy] * local_acc[3](dim) +
              _phi_map[2][qp_xy] *
                  (G4(dim, 0) * local_rot_acc[3](0) + G4(dim, 1) * local_rot_acc[3](1));

          _local_force[3](dim) +=
              _phi_map[3][qp_xy] * _phi_map[0][qp_xy] * local_acc[0](dim) +
              _phi_map[3][qp_xy] *
                  (G1(dim, 0) * local_rot_acc[0](0) + G1(dim, 1) * local_rot_acc[0](1)) +
              _phi_map[3][qp_xy] * _phi_map[1][qp_xy] * local_acc[1](dim) +
              _phi_map[3][qp_xy] *
                  (G2(dim, 0) * local_rot_acc[1](0) + G2(dim, 1) * local_rot_acc[1](1)) +
              _phi_map[3][qp_xy] * _phi_map[2][qp_xy] * local_acc[2](dim) +
              _phi_map[3][qp_xy] *
                  (G3(dim, 0) * local_rot_acc[2](0) + G3(dim, 1) * local_rot_acc[2](1)) +
              _phi_map[3][qp_xy] * _phi_map[3][qp_xy] * local_acc[3](dim) +
              _phi_map[3][qp_xy] *
                  (G4(dim, 0) * local_rot_acc[3](0) + G4(dim, 1) * local_rot_acc[3](1));
        }

        DenseVector<Real> momentInertia;
        momentInertia(0) =
            _phi_map[0][qp_xy] * local_acc[0](0) + _phi_map[1][qp_xy] * local_acc[1](0) +
            _phi_map[2][qp_xy] * local_acc[2](0) + _phi_map[3][qp_xy] * local_acc[3](0) +
            G1(0, 0) * local_rot_acc[0](0) + G1(0, 1) * local_rot_acc[0](1) +
            G2(0, 0) * local_rot_acc[1](0) + G2(0, 1) * local_rot_acc[1](1) +
            G3(0, 0) * local_rot_acc[2](0) + G3(0, 1) * local_rot_acc[2](1) +
            G4(0, 0) * local_rot_acc[3](0) + G4(0, 1) * local_rot_acc[3](1);

        momentInertia(1) =
            _phi_map[0][qp_xy] * local_acc[0](1) + _phi_map[1][qp_xy] * local_acc[1](1) +
            _phi_map[2][qp_xy] * local_acc[2](1) + _phi_map[3][qp_xy] * local_acc[3](1) +
            G1(1, 0) * local_rot_acc[0](0) + G1(1, 1) * local_rot_acc[0](1) +
            G2(1, 0) * local_rot_acc[1](0) + G2(1, 1) * local_rot_acc[1](1) +
            G3(1, 0) * local_rot_acc[2](0) + G3(1, 1) * local_rot_acc[2](1) +
            G4(1, 0) * local_rot_acc[3](0) + G4(1, 1) * local_rot_acc[3](1);

        momentInertia(2) =
            _phi_map[0][qp_xy] * local_acc[0](2) + _phi_map[1][qp_xy] * local_acc[1](2) +
            _phi_map[2][qp_xy] * local_acc[2](2) + _phi_map[3][qp_xy] * local_acc[3](2) +
            G1(2, 0) * local_rot_acc[0](0) + G1(2, 1) * local_rot_acc[0](1) +
            G2(2, 0) * local_rot_acc[1](0) + G2(2, 1) * local_rot_acc[1](1) +
            G3(2, 0) * local_rot_acc[2](0) + G3(2, 1) * local_rot_acc[2](1) +
            G4(2, 0) * local_rot_acc[3](0) + G4(2, 1) * local_rot_acc[3](1);

        _local_moment[0](0) += G1T(0, 0) * momentInertia(0) + G1T(0, 1) * momentInertia(1) +
                               G1T(0, 2) * momentInertia(2);

        _local_moment[0](1) += G1T(1, 0) * momentInertia(0) + G1T(1, 1) * momentInertia(1) +
                               G1T(1, 2) * momentInertia(2);

        _local_moment[1](0) += G1T(0, 0) * momentInertia(0) + G1T(0, 1) * momentInertia(1) +
                               G2T(0, 2) * momentInertia(2);

        _local_moment[1](1) += G1T(1, 0) * momentInertia(0) + G1T(1, 1) * momentInertia(1) +
                               G2T(1, 2) * momentInertia(2);

        _local_moment[2](0) += G1T(0, 0) * momentInertia(0) + G1T(0, 1) * momentInertia(1) +
                               G3T(0, 2) * momentInertia(2);

        _local_moment[2](1) += G1T(1, 0) * momentInertia(0) + G1T(1, 1) * momentInertia(1) +
                               G3T(1, 2) * momentInertia(2);

        _local_moment[3](0) += G1T(0, 0) * momentInertia(0) + G1T(0, 1) * momentInertia(1) +
                               G4T(0, 2) * momentInertia(2);

        _local_moment[3](1) += G1T(1, 0) * momentInertia(0) + G1T(1, 1) * momentInertia(1) +
                               G4T(1, 2) * momentInertia(2);
      }
    }

    // Global force and moments
    if (_component < 3)
    {
      _global_force_0 = _original_local_config[0] * _local_force[0];
      _global_force_1 = _original_local_config[0] * _local_force[1];
      _global_force_2 = _original_local_config[0] * _local_force[2];
      _global_force_3 = _original_local_config[0] * _local_force[3];
      _local_re(0) = _global_force_0(_component);
      _local_re(1) = _global_force_1(_component);
      _local_re(2) = _global_force_2(_component);
      _local_re(3) = _global_force_3(_component);
    }
    else
     // Only two rotational components: \alpha and \beta.
    {
      _global_moment_0 = _original_local_config[0] * _local_moment[0];
      _global_moment_1 = _original_local_config[0] * _local_moment[1];
      _global_moment_2 = _original_local_config[0] * _local_moment[2];
      _global_moment_3 = _original_local_config[0] * _local_moment[3];
      _local_re(0) = _global_moment_0(_component - 3);
      _local_re(1) = _global_moment_1(_component - 3);
      _local_re(2) = _global_moment_2(_component - 3);
      _local_re(3) = _global_moment_3(_component - 3);
    }

    accumulateTaggedLocalResidual();

    if (_has_save_in)
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      for (unsigned int i = 0; i < _save_in.size(); ++i)
        _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
    }
  }
  ADReal whatever;
  return whatever;
}
