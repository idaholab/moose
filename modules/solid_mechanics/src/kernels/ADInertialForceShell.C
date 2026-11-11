//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADInertialForceShell.h"

#include "SubProblem.h"
#include "libmesh/utility.h"
#include "MooseVariable.h"
#include "Assembly.h"
#include "NonlinearSystem.h"
#include "AuxiliarySystem.h"
#include "MooseMesh.h"

#include "libmesh/utility.h"
#include "libmesh/quadrature.h"
#include "libmesh/enum_quadrature_type.h"
#include "libmesh/fe_type.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature_gauss.h"
#include "MooseVariable.h"

registerMooseObject("SolidMechanicsApp", ADInertialForceShell);

InputParameters
ADInertialForceShell::validParams()
{
  InputParameters params = ADTimeKernel::validParams();

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
  params.addRequiredParam<Real>("thickness", "The kernel's thickness");
  params.addParam<MaterialPropertyName>("eta",
                                        0.0,
                                        "Name of material property or a constant real "
                                        "number defining the eta parameter for the "
                                        "Rayleigh damping.");
  params.addRangeCheckedParam<Real>("alpha",
                                    0.0,
                                    "alpha >= -0.3333 & alpha <= 0.0",
                                    "Alpha parameter for mass dependent numerical damping induced "
                                    "by HHT time integration scheme");

  return params;
}

ADInertialForceShell::ADInertialForceShell(const InputParameters & parameters)
  : ADTimeKernel(parameters),
    _nrot(coupledComponents("rotations")),
    _ndisp(coupledComponents("displacements")),
    _rot_num(_nrot),
    _disp_num(_ndisp),
    _vel_num(_ndisp),
    _accel_num(_ndisp),
    _rot_vel_num(_nrot),
    _rot_accel_num(_nrot),
    _component(getParam<unsigned int>("component")),
    _nodes(4),
    _v1(4),
    _v2(4),
    _node_normal(4),
    _eta(getMaterialProperty<Real>("eta")),
    _J_map(getADMaterialProperty<Real>("J_mapping_t_points_0")),
    _thickness(getParam<Real>("thickness")),
    _density(getMaterialProperty<Real>("density")),
    _alpha(getParam<Real>("alpha"))
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

  _x2(1) = 1;
  _x3(2) = 1;
}

void
ADInertialForceShell::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  precalculateResidual();

  /* ----------------------------------------------------- */

  if (_dt != 0.0)
  {
    computeShellInertialForces(_ad_coord, _ad_JxW);

    if (_component < 3)
    {
      _local_re(0) = raw_value(_thickness * _nodal_force[0](_component));
      _local_re(1) = raw_value(_thickness * _nodal_force[1](_component));
      _local_re(2) = raw_value(_thickness * _nodal_force[2](_component));
      _local_re(3) = raw_value(_thickness * _nodal_force[3](_component));
    }
    else
    {
      _local_re(0) = raw_value(_nodal_moment[0](_component - 3));
      _local_re(1) = raw_value(_nodal_moment[1](_component - 3));
      _local_re(2) = raw_value(_nodal_moment[2](_component - 3));
      _local_re(3) = raw_value(_nodal_moment[3](_component - 3));
    }
  }

  accumulateTaggedLocalResidual();

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i = 0; i < _save_in.size(); i++)
      _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }
}

void
ADInertialForceShell::computeResidualsForJacobian()
{
  if (_residuals.size() != _test.size())
    _residuals.resize(_test.size(), 0);
  for (auto & r : _residuals)
    r = 0;

  precalculateResidual();

  mooseAssert(_residuals.size() >= 4,
              "This is hard coded to index from 0 to 3, so we must have at least four spots in our "
              "container. I'd prefer to assert that the size == 4, but I don't know what the "
              "tensor mechanics folks expect.");

  if (_dt != 0.0)
  {
    computeShellInertialForces(_ad_coord, _ad_JxW);

    if (_component < 3)
    {
      _residuals[0] = _thickness * _nodal_force[0](_component);
      _residuals[1] = _thickness * _nodal_force[1](_component);
      _residuals[2] = _thickness * _nodal_force[2](_component);
      _residuals[3] = _thickness * _nodal_force[3](_component);
    }
    else
    {
      _residuals[0] = _nodal_moment[0](_component - 3);
      _residuals[1] = _nodal_moment[1](_component - 3);
      _residuals[2] = _nodal_moment[2](_component - 3);
      _residuals[3] = _nodal_moment[3](_component - 3);
    }
  }
}

void
ADInertialForceShell::computeShellInertialForces(const MooseArray<ADReal> & _ad_coord,
                                                 const MooseArray<ADReal> & _ad_JxW)
{
  // Loosely following notation in: "On finite element nonlinear analysis of general shell
  // structures", PhD thesis by Said Bolourchi (1975).

  _nodes[0] = _current_elem->node_ptr(0);
  _nodes[1] = _current_elem->node_ptr(1);
  _nodes[2] = _current_elem->node_ptr(2);
  _nodes[3] = _current_elem->node_ptr(3);

  ADRealVectorValue x = (*_nodes[1] - *_nodes[0]);
  ADRealVectorValue y = (*_nodes[3] - *_nodes[0]);
  ADRealVectorValue normal = x.cross(y);
  normal /= normal.norm();

  _node_normal[0] = normal;
  _node_normal[1] = normal;
  _node_normal[2] = normal;
  _node_normal[3] = normal;

  // compute nodal local axis
  for (unsigned int k = 0; k < _nodes.size(); ++k)
  {
    _v1[k] = _x2.cross(_node_normal[k]);

    // If x2 is parallel to node normal, set V1 to x3
    if (MooseUtils::absoluteFuzzyEqual(_v1[k].norm(), 0.0, 1e-6))
      _v1[k] = _x3;

    _v1[k] /= _v1[k].norm();

    _v2[k] = _node_normal[k].cross(_v1[k]);
  }

  NonlinearSystemBase & nonlinear_sys = _fe_problem.getNonlinearSystemBase(_sys.number());

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
  const NumericVector<Number> & old_vel = *nonlinear_sys.solutionUDotOld();
  const NumericVector<Number> & accel = *nonlinear_sys.solutionUDotDot();

  // The translational velocities and accelerations are global components. The rotational ones are
  // rotations about the in-plane local axes of the shell strain material. The current velocity and
  // acceleration carry their derivatives with respect to their own dof, so that the AD Jacobian
  // includes the mass and damping terms. The old velocity does not depend on the current solution
  // and carries no derivative.
  for (const auto i : make_range(_ndisp))
    for (const auto k : make_range(4))
    {
      // translational velocities and accelerations
      const auto dof_index = _nodes[k]->dof_number(nonlinear_sys.number(), _disp_num[i], 0);
      _vel.pos[k](i) = vel(dof_index);
      _old_vel.pos[k](i) = old_vel(dof_index);
      _accel.pos[k](i) = accel(dof_index);
      if (ADReal::do_derivatives)
      {
        Moose::derivInsert(_accel.pos[k](i).derivatives(), dof_index, nonlinear_sys.duDotDotDu());
        Moose::derivInsert(
            _vel.pos[k](i).derivatives(), dof_index, nonlinear_sys.duDotDu(_disp_num[i]));
      }
    }

  for (const auto i : make_range(_nrot))
    for (const auto k : make_range(4))
    {
      // rotational velocities and accelerations
      const auto dof_index = _nodes[k]->dof_number(nonlinear_sys.number(), _rot_num[i], 0);
      _vel.rot[k](i) = vel(dof_index);
      _old_vel.rot[k](i) = old_vel(dof_index);
      _accel.rot[k](i) = accel(dof_index);
      if (ADReal::do_derivatives)
      {
        Moose::derivInsert(_accel.rot[k](i).derivatives(), dof_index, nonlinear_sys.duDotDotDu());
        Moose::derivInsert(
            _vel.rot[k](i).derivatives(), dof_index, nonlinear_sys.duDotDu(_rot_num[i]));
      }
    }

  // Conversions to ADDenseVector from ADRealVectorValue: Make a method out of this.
  ADDenseVector local_accel_dv_0(3);
  ADDenseVector local_accel_dv_1(3);
  ADDenseVector local_accel_dv_2(3);
  ADDenseVector local_accel_dv_3(3);

  ADDenseVector local_rot_accel_dv_0(3);
  ADDenseVector local_rot_accel_dv_1(3);
  ADDenseVector local_rot_accel_dv_2(3);
  ADDenseVector local_rot_accel_dv_3(3);

  ADDenseVector local_vel_dv_0(3);
  ADDenseVector local_vel_dv_1(3);
  ADDenseVector local_vel_dv_2(3);
  ADDenseVector local_vel_dv_3(3);

  ADDenseVector local_rot_vel_dv_0(3);
  ADDenseVector local_rot_vel_dv_1(3);
  ADDenseVector local_rot_vel_dv_2(3);
  ADDenseVector local_rot_vel_dv_3(3);

  ADDenseVector local_old_vel_dv_0(3);
  ADDenseVector local_old_vel_dv_1(3);
  ADDenseVector local_old_vel_dv_2(3);
  ADDenseVector local_old_vel_dv_3(3);

  ADDenseVector local_old_rot_vel_dv_0(3);
  ADDenseVector local_old_rot_vel_dv_1(3);
  ADDenseVector local_old_rot_vel_dv_2(3);
  ADDenseVector local_old_rot_vel_dv_3(3);

  for (unsigned int i = 0; i < 3; i++)
  {
    local_accel_dv_0(i) = _accel.pos[0](i);
    local_accel_dv_1(i) = _accel.pos[1](i);
    local_accel_dv_2(i) = _accel.pos[2](i);
    local_accel_dv_3(i) = _accel.pos[3](i);

    local_rot_accel_dv_0(i) = _accel.rot[0](i);
    local_rot_accel_dv_1(i) = _accel.rot[1](i);
    local_rot_accel_dv_2(i) = _accel.rot[2](i);
    local_rot_accel_dv_3(i) = _accel.rot[3](i);

    local_vel_dv_0(i) = _vel.pos[0](i);
    local_vel_dv_1(i) = _vel.pos[1](i);
    local_vel_dv_2(i) = _vel.pos[2](i);
    local_vel_dv_3(i) = _vel.pos[3](i);

    local_rot_vel_dv_0(i) = _vel.rot[0](i);
    local_rot_vel_dv_1(i) = _vel.rot[1](i);
    local_rot_vel_dv_2(i) = _vel.rot[2](i);
    local_rot_vel_dv_3(i) = _vel.rot[3](i);

    local_old_vel_dv_0(i) = _old_vel.pos[0](i);
    local_old_vel_dv_1(i) = _old_vel.pos[1](i);
    local_old_vel_dv_2(i) = _old_vel.pos[2](i);
    local_old_vel_dv_3(i) = _old_vel.pos[3](i);

    local_old_rot_vel_dv_0(i) = _old_vel.rot[0](i);
    local_old_rot_vel_dv_1(i) = _old_vel.rot[1](i);
    local_old_rot_vel_dv_2(i) = _old_vel.rot[2](i);
    local_old_rot_vel_dv_3(i) = _old_vel.rot[3](i);
  }
  unsigned int dim = _current_elem->dim();

  // Nodal G vectors, G_k = (h/2) [-v2_k, v1_k], with h the thickness. The rotary inertia needs
  // only the products G_i^T G_j. The rotation variables are defined about the local axes of the
  // shell strain material, which can differ from v1 and v2 by an in-plane rotation. All four nodes
  // share the element normal, so G_i^T G_j = (h/2)^2 I for every orthonormal in-plane pair of
  // axes, and that difference does not change the result.
  _0g1_vectors[0] = -0.5 * _thickness * _v2[0];
  _0g1_vectors[1] = 0.5 * _thickness * _v1[0];

  _0g2_vectors[0] = -0.5 * _thickness * _v2[1];
  _0g2_vectors[1] = 0.5 * _thickness * _v1[1];

  _0g3_vectors[0] = -0.5 * _thickness * _v2[2];
  _0g3_vectors[1] = 0.5 * _thickness * _v1[2];

  _0g4_vectors[0] = -0.5 * _thickness * _v2[3];
  _0g4_vectors[1] = 0.5 * _thickness * _v1[3];

  // Columns of G for the four nodes
  const std::array<std::array<ADRealVectorValue, 2>, 4> g = {
      {_0g1_vectors, _0g2_vectors, _0g3_vectors, _0g4_vectors}};

  std::vector<ADDenseVector> local_acc;
  local_acc.resize(4);
  local_acc[0].resize(3);
  local_acc[1].resize(3);
  local_acc[2].resize(3);
  local_acc[3].resize(3);

  local_acc[0] = local_accel_dv_0;
  local_acc[1] = local_accel_dv_1;
  local_acc[2] = local_accel_dv_2;
  local_acc[3] = local_accel_dv_3;

  std::vector<ADDenseVector> local_rot_acc;
  local_rot_acc.resize(4);
  local_rot_acc[0].resize(3);
  local_rot_acc[1].resize(3);
  local_rot_acc[2].resize(3);
  local_rot_acc[3].resize(3);
  local_rot_acc[0] = local_rot_accel_dv_0;
  local_rot_acc[1] = local_rot_accel_dv_1;
  local_rot_acc[2] = local_rot_accel_dv_2;
  local_rot_acc[3] = local_rot_accel_dv_3;

  // HHT-weighted velocity for Rayleigh damping, (1 + alpha) v - alpha v_old
  // {
  std::vector<ADDenseVector> local_vel;
  local_vel.resize(4);
  local_vel[0].resize(3);
  local_vel[1].resize(3);
  local_vel[2].resize(3);
  local_vel[3].resize(3);

  local_vel_dv_0.scale(1 + _alpha);
  local_old_vel_dv_0.scale(-_alpha);
  local_vel_dv_1.scale(1 + _alpha);
  local_old_vel_dv_1.scale(-_alpha);
  local_vel_dv_2.scale(1 + _alpha);
  local_old_vel_dv_2.scale(-_alpha);
  local_vel_dv_3.scale(1 + _alpha);
  local_old_vel_dv_3.scale(-_alpha);

  local_vel_dv_0.add(1.0, local_old_vel_dv_0);
  local_vel_dv_1.add(1.0, local_old_vel_dv_1);
  local_vel_dv_2.add(1.0, local_old_vel_dv_2);
  local_vel_dv_3.add(1.0, local_old_vel_dv_3);

  local_vel[0] = local_vel_dv_0;
  local_vel[1] = local_vel_dv_1;
  local_vel[2] = local_vel_dv_2;
  local_vel[3] = local_vel_dv_3;

  std::vector<ADDenseVector> local_rot_vel;
  local_rot_vel.resize(4);
  local_rot_vel[0].resize(3);
  local_rot_vel[1].resize(3);
  local_rot_vel[2].resize(3);
  local_rot_vel[3].resize(3);

  local_rot_vel_dv_0.scale(1 + _alpha);
  local_old_rot_vel_dv_0.scale(-_alpha);
  local_rot_vel_dv_1.scale(1 + _alpha);
  local_old_rot_vel_dv_1.scale(-_alpha);
  local_rot_vel_dv_2.scale(1 + _alpha);
  local_old_rot_vel_dv_2.scale(-_alpha);
  local_rot_vel_dv_3.scale(1 + _alpha);
  local_old_rot_vel_dv_3.scale(-_alpha);

  local_rot_vel_dv_0.add(1.0, local_old_rot_vel_dv_0);
  local_rot_vel_dv_1.add(1.0, local_old_rot_vel_dv_1);
  local_rot_vel_dv_2.add(1.0, local_old_rot_vel_dv_2);
  local_rot_vel_dv_3.add(1.0, local_old_rot_vel_dv_3);

  local_rot_vel[0] = local_rot_vel_dv_0;
  local_rot_vel[1] = local_rot_vel_dv_1;
  local_rot_vel[2] = local_rot_vel_dv_2;
  local_rot_vel[3] = local_rot_vel_dv_3;
  // }

  FEType fe_type(Utility::string_to_enum<Order>("First"),
                 Utility::string_to_enum<FEFamily>("LAGRANGE"));
  fe_type.set_p_refinement(false);
  auto & fe = _fe_problem.assembly(_tid, _sys.number()).getFE(fe_type, dim);
  _dphidxi_map = fe->get_fe_map().get_dphidxi_map();
  _dphideta_map = fe->get_fe_map().get_dphideta_map();
  _phi_map = fe->get_fe_map().get_phi_map();

  // quadrature points in isoparametric space
  _2d_points = _qrule->get_points(); // would be in 2D

  std::vector<const Node *> nodes;
  for (unsigned int i = 0; i < 4; ++i)
    nodes.push_back(_current_elem->node_ptr(i));

  for (unsigned int i = 0; i < _ndisp; i++)
    for (unsigned int j = 0; j < 4; j++)
      _nodal_force[j](i) = 0.0;

  for (unsigned int i = 0; i < _nrot; i++)
    for (unsigned int j = 0; j < 4; j++)
      _nodal_moment[j](i) = 0.0;

  // The thickness integral is int_{-1}^{1} zeta^2 (h/2) dzeta = h/3, with zeta the isoparametric
  // thickness coordinate. With the factor (h/2)^2 of G_i^T G_j, the rotary inertia per unit area
  // is rho h^3 / 12.
  const Real rot_thickness = raw_value(_thickness) / 3.0;

  // G_i^T G_j (theta_ddot_j + eta theta_dot*_j) for each node pair (i, j), with theta_dot* the
  // HHT-weighted rotational velocity. It does not vary over the element.
  std::array<std::array<std::array<ADReal, 2>, 4>, 4> rot_rate;
  for (const auto i : make_range(4))
    for (const auto j : make_range(4))
      for (const auto a : make_range(2))
      {
        rot_rate[i][j][a] = 0.0;
        for (const auto b : make_range(2))
          rot_rate[i][j][a] +=
              (g[i][a] * g[j][b]) * (local_rot_acc[j](b) + _eta[0] * local_rot_vel[j](b));
      }

  for (unsigned int qp_xy = 0; qp_xy < _2d_points.size(); ++qp_xy)
  {
    ADReal factor_qxy = _ad_coord[qp_xy] * _ad_JxW[qp_xy] * _density[qp_xy];

    // Account for inertia on displacement degrees of freedom
    for (unsigned int dim = 0; dim < 3; dim++)
    {
      _nodal_force[0](dim) +=
          factor_qxy * (_phi_map[0][qp_xy] * _phi_map[0][qp_xy] * local_acc[0](dim) +
                        _phi_map[0][qp_xy] * _phi_map[1][qp_xy] * local_acc[1](dim) +
                        _phi_map[0][qp_xy] * _phi_map[2][qp_xy] * local_acc[2](dim) +
                        _phi_map[0][qp_xy] * _phi_map[3][qp_xy] * local_acc[3](dim));

      _nodal_force[0](dim) += factor_qxy * _eta[0] *
                              (_phi_map[0][qp_xy] * _phi_map[0][qp_xy] * local_vel[0](dim) +
                               _phi_map[0][qp_xy] * _phi_map[1][qp_xy] * local_vel[1](dim) +
                               _phi_map[0][qp_xy] * _phi_map[2][qp_xy] * local_vel[2](dim) +
                               _phi_map[0][qp_xy] * _phi_map[3][qp_xy] * local_vel[3](dim));

      _nodal_force[1](dim) +=
          factor_qxy * (_phi_map[1][qp_xy] * _phi_map[0][qp_xy] * local_acc[0](dim) +
                        _phi_map[1][qp_xy] * _phi_map[1][qp_xy] * local_acc[1](dim) +
                        _phi_map[1][qp_xy] * _phi_map[2][qp_xy] * local_acc[2](dim) +
                        _phi_map[1][qp_xy] * _phi_map[3][qp_xy] * local_acc[3](dim));

      _nodal_force[1](dim) += factor_qxy * _eta[0] *
                              (_phi_map[1][qp_xy] * _phi_map[0][qp_xy] * local_vel[0](dim) +
                               _phi_map[1][qp_xy] * _phi_map[1][qp_xy] * local_vel[1](dim) +
                               _phi_map[1][qp_xy] * _phi_map[2][qp_xy] * local_vel[2](dim) +
                               _phi_map[1][qp_xy] * _phi_map[3][qp_xy] * local_vel[3](dim));
      _nodal_force[2](dim) +=
          factor_qxy * (_phi_map[2][qp_xy] * _phi_map[0][qp_xy] * local_acc[0](dim) +
                        _phi_map[2][qp_xy] * _phi_map[1][qp_xy] * local_acc[1](dim) +
                        _phi_map[2][qp_xy] * _phi_map[2][qp_xy] * local_acc[2](dim) +
                        _phi_map[2][qp_xy] * _phi_map[3][qp_xy] * local_acc[3](dim));

      _nodal_force[2](dim) += factor_qxy * _eta[0] *
                              (_phi_map[2][qp_xy] * _phi_map[0][qp_xy] * local_vel[0](dim) +
                               _phi_map[2][qp_xy] * _phi_map[1][qp_xy] * local_vel[1](dim) +
                               _phi_map[2][qp_xy] * _phi_map[2][qp_xy] * local_vel[2](dim) +
                               _phi_map[2][qp_xy] * _phi_map[3][qp_xy] * local_vel[3](dim));

      _nodal_force[3](dim) +=
          factor_qxy * (_phi_map[3][qp_xy] * _phi_map[0][qp_xy] * local_acc[0](dim) +
                        _phi_map[3][qp_xy] * _phi_map[1][qp_xy] * local_acc[1](dim) +
                        _phi_map[3][qp_xy] * _phi_map[2][qp_xy] * local_acc[2](dim) +
                        _phi_map[3][qp_xy] * _phi_map[3][qp_xy] * local_acc[3](dim));

      _nodal_force[3](dim) += factor_qxy * _eta[0] *
                              (_phi_map[3][qp_xy] * _phi_map[0][qp_xy] * local_vel[0](dim) +
                               _phi_map[3][qp_xy] * _phi_map[1][qp_xy] * local_vel[1](dim) +
                               _phi_map[3][qp_xy] * _phi_map[2][qp_xy] * local_vel[2](dim) +
                               _phi_map[3][qp_xy] * _phi_map[3][qp_xy] * local_vel[3](dim));
    }

    // Account for inertia on rotational degrees of freedom
    for (const auto i : make_range(4))
      for (const auto j : make_range(4))
      {
        const ADReal weight = factor_qxy * rot_thickness * _phi_map[i][qp_xy] * _phi_map[j][qp_xy];
        for (const auto a : make_range(2))
          _nodal_moment[i](a) += weight * rot_rate[i][j][a];
      }
  }
}
