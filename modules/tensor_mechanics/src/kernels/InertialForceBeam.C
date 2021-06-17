//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InertialForceBeam.h"
#include "SubProblem.h"
#include "libmesh/utility.h"
#include "MooseVariable.h"
#include "Assembly.h"
#include "NonlinearSystem.h"
#include "AuxiliarySystem.h"
#include "MooseMesh.h"

registerMooseObject("TensorMechanicsApp", InertialForceBeam);

InputParameters
InertialForceBeam::validParams()
{
  InputParameters params = TimeKernel::validParams();
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
  params.addRangeCheckedParam<Real>(
      "beta", "beta>0.0", "beta parameter for Newmark Time integration");
  params.addRangeCheckedParam<Real>(
      "gamma", "gamma>0.0", "gamma parameter for Newmark Time integration");
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
  params.addParam<MaterialPropertyName>(
      "density",
      "density",
      "Name of Material Property  or a constant real number defining the density of the beam.");
  params.addRequiredCoupledVar("area", "Variable containing cross-section area");
  params.addCoupledVar("Ay", 0.0, "Variable containing first moment of area about y axis");
  params.addCoupledVar("Az", 0.0, "Variable containing first moment of area about z axis");
  params.addCoupledVar("Ix",
                       "Variable containing second moment of area about x axis. Defaults to Iy+Iz");
  params.addRequiredCoupledVar("Iy", "Variable containing second moment of area about y axis");
  params.addRequiredCoupledVar("Iz", "Variable containing second moment of area about z axis");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "component",
      "component<6",
      "An integer corresponding to the direction "
      "the variable this kernel acts in. (0 for disp_x, "
      "1 for disp_y, 2 for disp_z, 3 for rot_x, 4 for rot_y and 5 for rot_z)");
  return params;
}

InertialForceBeam::InertialForceBeam(const InputParameters & parameters)
  : TimeKernel(parameters),
    _has_beta(isParamValid("beta")),
    _has_gamma(isParamValid("gamma")),
    _has_velocities(isParamValid("velocities")),
    _has_rot_velocities(isParamValid("rotational_velocities")),
    _has_accelerations(isParamValid("accelerations")),
    _has_rot_accelerations(isParamValid("rotational_accelerations")),
    _has_Ix(isParamValid("Ix")),
    _density(getMaterialProperty<Real>("density")),
    _nrot(coupledComponents("rotations")),
    _ndisp(coupledComponents("displacements")),
    _rot_num(_nrot),
    _disp_num(_ndisp),
    _vel_num(_ndisp),
    _accel_num(_ndisp),
    _rot_vel_num(_nrot),
    _rot_accel_num(_nrot),
    _area(coupledValue("area")),
    _Ay(coupledValue("Ay")),
    _Az(coupledValue("Az")),
    _Ix(_has_Ix ? coupledValue("Ix") : _zero),
    _Iy(coupledValue("Iy")),
    _Iz(coupledValue("Iz")),
    _beta(_has_beta ? getParam<Real>("beta") : 0.1),
    _gamma(_has_gamma ? getParam<Real>("gamma") : 0.1),
    _eta(getMaterialProperty<Real>("eta")),
    _alpha(getParam<Real>("alpha")),
    _original_local_config(getMaterialPropertyByName<RankTwoTensor>("initial_rotation")),
    _original_length(getMaterialPropertyByName<Real>("original_length")),
    _component(getParam<unsigned int>("component")),
    _local_force(2),
    _local_moment(2)
{
  // Checking for consistency between the length of the provided rotations and displacements vector
  if (_ndisp != _nrot)
    mooseError("InertialForceBeam: The number of variables supplied in 'displacements' and "
               "'rotations' must match.");

  if (_has_beta && _has_gamma && _has_velocities && _has_accelerations && _has_rot_velocities &&
      _has_rot_accelerations)
  {
    if ((coupledComponents("velocities") != _ndisp) ||
        (coupledComponents("accelerations") != _ndisp) ||
        (coupledComponents("rotational_velocities") != _ndisp) ||
        (coupledComponents("rotational_accelerations") != _ndisp))
      mooseError(
          "InertialForceBeam: The number of variables supplied in 'velocities', "
          "'accelerations', 'rotational_velocities' and 'rotational_accelerations' must match "
          "the number of displacement variables.");

    // fetch coupled velocities and accelerations
    for (unsigned int i = 0; i < _ndisp; ++i)
    {
      MooseVariable * vel_variable = getVar("velocities", i);
      _vel_num[i] = vel_variable->number();

      MooseVariable * accel_variable = getVar("accelerations", i);
      _accel_num[i] = accel_variable->number();

      MooseVariable * rot_vel_variable = getVar("rotational_velocities", i);
      _rot_vel_num[i] = rot_vel_variable->number();

      MooseVariable * rot_accel_variable = getVar("rotational_accelerations", i);
      _rot_accel_num[i] = rot_accel_variable->number();
    }
  }
  else if (!_has_beta && !_has_gamma && !_has_velocities && !_has_accelerations &&
           !_has_rot_velocities && !_has_rot_accelerations)
  {
    _du_dot_du = &coupledDotDu("displacements", 0);
    _du_dotdot_du = &coupledDotDotDu("displacements", 0);
  }
  else
    mooseError("InertialForceBeam: Either all or none of `beta`, `gamma`, `velocities`, "
               "`accelerations`, `rotational_velocities` and `rotational_accelerations` should be "
               "provided as input.");

  // fetch coupled displacements and rotations
  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    MooseVariable * disp_variable = getVar("displacements", i);
    _disp_num[i] = disp_variable->number();

    MooseVariable * rot_variable = getVar("rotations", i);
    _rot_num[i] = rot_variable->number();
  }
}

void
InertialForceBeam::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  if (_dt != 0.0)
  {
    // fetch the two end nodes for _current_elem
    std::vector<const Node *> node;
    for (unsigned int i = 0; i < 2; ++i)
      node.push_back(_current_elem->node_ptr(i));

    // Fetch the solution for the two end nodes at time t
    NonlinearSystemBase & nonlinear_sys = _fe_problem.getNonlinearSystemBase();

    if (_has_beta)
    {
      const NumericVector<Number> & sol = *nonlinear_sys.currentSolution();
      const NumericVector<Number> & sol_old = nonlinear_sys.solutionOld();

      AuxiliarySystem & aux = _fe_problem.getAuxiliarySystem();
      const NumericVector<Number> & aux_sol_old = aux.solutionOld();

      mooseAssert(_beta > 0.0, "InertialForceBeam: Beta parameter should be positive.");
      mooseAssert(_eta[0] >= 0.0, "InertialForceBeam: Eta parameter should be non-negative.");

      for (unsigned int i = 0; i < _ndisp; ++i)
      {
        // obtain delta displacement
        unsigned int dof_index_0 = node[0]->dof_number(nonlinear_sys.number(), _disp_num[i], 0);
        unsigned int dof_index_1 = node[1]->dof_number(nonlinear_sys.number(), _disp_num[i], 0);
        const Real disp_0 = sol(dof_index_0) - sol_old(dof_index_0);
        const Real disp_1 = sol(dof_index_1) - sol_old(dof_index_1);

        dof_index_0 = node[0]->dof_number(nonlinear_sys.number(), _rot_num[i], 0);
        dof_index_1 = node[1]->dof_number(nonlinear_sys.number(), _rot_num[i], 0);
        const Real rot_0 = sol(dof_index_0) - sol_old(dof_index_0);
        const Real rot_1 = sol(dof_index_1) - sol_old(dof_index_1);

        // obtain new translational and rotational velocities and accelerations using newmark-beta
        // time integration
        _vel_old_0(i) = aux_sol_old(node[0]->dof_number(aux.number(), _vel_num[i], 0));
        _vel_old_1(i) = aux_sol_old(node[1]->dof_number(aux.number(), _vel_num[i], 0));
        const Real accel_old_0 = aux_sol_old(node[0]->dof_number(aux.number(), _accel_num[i], 0));
        const Real accel_old_1 = aux_sol_old(node[1]->dof_number(aux.number(), _accel_num[i], 0));

        _rot_vel_old_0(i) = aux_sol_old(node[0]->dof_number(aux.number(), _rot_vel_num[i], 0));
        _rot_vel_old_1(i) = aux_sol_old(node[1]->dof_number(aux.number(), _rot_vel_num[i], 0));
        const Real rot_accel_old_0 =
            aux_sol_old(node[0]->dof_number(aux.number(), _rot_accel_num[i], 0));
        const Real rot_accel_old_1 =
            aux_sol_old(node[1]->dof_number(aux.number(), _rot_accel_num[i], 0));

        _accel_0(i) =
            1. / _beta * (disp_0 / (_dt * _dt) - _vel_old_0(i) / _dt - accel_old_0 * (0.5 - _beta));
        _accel_1(i) =
            1. / _beta * (disp_1 / (_dt * _dt) - _vel_old_1(i) / _dt - accel_old_1 * (0.5 - _beta));
        _rot_accel_0(i) =
            1. / _beta *
            (rot_0 / (_dt * _dt) - _rot_vel_old_0(i) / _dt - rot_accel_old_0 * (0.5 - _beta));
        _rot_accel_1(i) =
            1. / _beta *
            (rot_1 / (_dt * _dt) - _rot_vel_old_1(i) / _dt - rot_accel_old_1 * (0.5 - _beta));

        _vel_0(i) = _vel_old_0(i) + (_dt * (1 - _gamma)) * accel_old_0 + _gamma * _dt * _accel_0(i);
        _vel_1(i) = _vel_old_1(i) + (_dt * (1 - _gamma)) * accel_old_1 + _gamma * _dt * _accel_1(i);
        _rot_vel_0(i) = _rot_vel_old_0(i) + (_dt * (1 - _gamma)) * rot_accel_old_0 +
                        _gamma * _dt * _rot_accel_0(i);
        _rot_vel_1(i) = _rot_vel_old_1(i) + (_dt * (1 - _gamma)) * rot_accel_old_1 +
                        _gamma * _dt * _rot_accel_1(i);
      }
    }
    else
    {
      if (!nonlinear_sys.solutionUDot())
        mooseError("InertialForceBeam: Time derivative of solution (`u_dot`) is not stored. Please "
                   "set uDotRequested() to true in FEProblemBase before requesting `u_dot`.");

      if (!nonlinear_sys.solutionUDotOld())
        mooseError("InertialForceBeam: Old time derivative of solution (`u_dot_old`) is not "
                   "stored. Please set uDotOldRequested() to true in FEProblemBase before "
                   "requesting `u_dot_old`.");

      if (!nonlinear_sys.solutionUDotDot())
        mooseError("InertialForceBeam: Second time derivative of solution (`u_dotdot`) is not "
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
        _vel_0(i) = vel(dof_index_0);
        _vel_1(i) = vel(dof_index_1);
        _vel_old_0(i) = vel_old(dof_index_0);
        _vel_old_1(i) = vel_old(dof_index_1);
        _accel_0(i) = accel(dof_index_0);
        _accel_1(i) = accel(dof_index_1);

        // rotational velocities and accelerations
        dof_index_0 = node[0]->dof_number(nonlinear_sys.number(), _rot_num[i], 0);
        dof_index_1 = node[1]->dof_number(nonlinear_sys.number(), _rot_num[i], 0);
        _rot_vel_0(i) = vel(dof_index_0);
        _rot_vel_1(i) = vel(dof_index_1);
        _rot_vel_old_0(i) = vel_old(dof_index_0);
        _rot_vel_old_1(i) = vel_old(dof_index_1);
        _rot_accel_0(i) = accel(dof_index_0);
        _rot_accel_1(i) = accel(dof_index_1);
      }
    }

    // transform translational and rotational velocities and accelerations to the initial local
    // configuration of the beam
    _local_vel_old_0 = _original_local_config[0] * _vel_old_0;
    _local_vel_old_1 = _original_local_config[0] * _vel_old_1;
    _local_vel_0 = _original_local_config[0] * _vel_0;
    _local_vel_1 = _original_local_config[0] * _vel_1;
    _local_accel_0 = _original_local_config[0] * _accel_0;
    _local_accel_1 = _original_local_config[0] * _accel_1;

    _local_rot_vel_old_0 = _original_local_config[0] * _rot_vel_old_0;
    _local_rot_vel_old_1 = _original_local_config[0] * _rot_vel_old_1;
    _local_rot_vel_0 = _original_local_config[0] * _rot_vel_0;
    _local_rot_vel_1 = _original_local_config[0] * _rot_vel_1;
    _local_rot_accel_0 = _original_local_config[0] * _rot_accel_0;
    _local_rot_accel_1 = _original_local_config[0] * _rot_accel_1;

    // local residual
    for (unsigned int i = 0; i < _ndisp; ++i)
    {
      if (_component < 3)
      {
        _local_force[0](i) = _density[0] * _area[0] * _original_length[0] / 3.0 *
                             (_local_accel_0(i) + _local_accel_1(i) / 2.0 +
                              _eta[0] * (1.0 + _alpha) * (_local_vel_0(i) + _local_vel_1(i) / 2.0) -
                              _alpha * _eta[0] * (_local_vel_old_0(i) + _local_vel_old_1(i) / 2.0));
        _local_force[1](i) = _density[0] * _area[0] * _original_length[0] / 3.0 *
                             (_local_accel_1(i) + _local_accel_0(i) / 2.0 +
                              _eta[0] * (1.0 + _alpha) * (_local_vel_1(i) + _local_vel_0(i) / 2.0) -
                              _alpha * _eta[0] * (_local_vel_old_1(i) + _local_vel_old_0(i) / 2.0));
      }

      if (_component > 2)
      {
        Real I = _Iy[0] + _Iz[0];
        if (_has_Ix && (i == 0))
          I = _Ix[0];
        if (i == 1)
          I = _Iz[0];
        else if (i == 2)
          I = _Iy[0];

        _local_moment[0](i) =
            _density[0] * I * _original_length[0] / 3.0 *
            (_local_rot_accel_0(i) + _local_rot_accel_1(i) / 2.0 +
             _eta[0] * (1.0 + _alpha) * (_local_rot_vel_0(i) + _local_rot_vel_1(i) / 2.0) -
             _alpha * _eta[0] * (_local_rot_vel_old_0(i) + _local_rot_vel_old_1(i) / 2.0));
        _local_moment[1](i) =
            _density[0] * I * _original_length[0] / 3.0 *
            (_local_rot_accel_1(i) + _local_rot_accel_0(i) / 2.0 +
             _eta[0] * (1.0 + _alpha) * (_local_rot_vel_1(i) + _local_rot_vel_0(i) / 2.0) -
             _alpha * _eta[0] * (_local_rot_vel_old_1(i) + _local_rot_vel_old_0(i) / 2.0));
      }
    }

    // If Ay or Az are non-zero, contribution of rotational accelerations to translational forces
    // and vice versa have to be added
    if (_component < 3)
    {
      _local_force[0](0) +=
          _density[0] * _original_length[0] / 3.0 *
          (_Az[0] * (_local_rot_accel_0(1) + _local_rot_accel_1(1) / 2.0 +
                     _eta[0] * (1.0 + _alpha) * (_local_rot_vel_0(1) + _local_rot_vel_1(1) / 2.0) -
                     _alpha * _eta[0] * (_local_rot_vel_old_0(1) + _local_rot_vel_old_1(1) / 2.0)) -
           _Ay[0] * (_local_rot_accel_0(2) + _local_rot_accel_1(2) / 2.0 +
                     _eta[0] * (1.0 + _alpha) * (_local_rot_vel_0(2) + _local_rot_vel_1(2) / 2.0) -
                     _alpha * _eta[0] * (_local_rot_vel_old_0(2) + _local_rot_vel_old_1(2) / 2.0)));
      _local_force[1](0) +=
          _density[0] * _original_length[0] / 3.0 *
          (_Az[0] * (_local_rot_accel_1(1) + _local_rot_accel_0(1) / 2.0 +
                     _eta[0] * (1.0 + _alpha) * (_local_rot_vel_1(1) + _local_rot_vel_0(1) / 2.0) -
                     _alpha * _eta[0] * (_local_rot_vel_old_1(1) + _local_rot_vel_old_0(1) / 2.0)) -
           _Ay[0] * (_local_rot_accel_1(2) + _local_rot_accel_0(2) / 2.0 +
                     _eta[0] * (1.0 + _alpha) * (_local_rot_vel_1(2) + _local_rot_vel_0(2) / 2.0) -
                     _alpha * _eta[0] * (_local_rot_vel_old_1(2) + _local_rot_vel_old_0(2) / 2.0)));

      _local_force[0](1) +=
          -_density[0] * _original_length[0] / 3.0 * _Az[0] *
          (_local_rot_accel_0(0) + _local_rot_accel_1(0) / 2.0 +
           _eta[0] * (1.0 + _alpha) * (_local_rot_vel_0(0) + _local_rot_vel_1(0) / 2.0) -
           _alpha * _eta[0] * (_local_rot_vel_old_0(0) + _local_rot_vel_old_1(0) / 2.0));
      _local_force[1](1) +=
          -_density[0] * _original_length[0] / 3.0 * _Az[0] *
          (_local_rot_accel_1(0) + _local_rot_accel_0(0) / 2.0 +
           _eta[0] * (1.0 + _alpha) * (_local_rot_vel_1(0) + _local_rot_vel_0(0) / 2.0) -
           _alpha * _eta[0] * (_local_rot_vel_old_1(0) + _local_rot_vel_old_0(0) / 2.0));

      _local_force[0](2) +=
          _density[0] * _original_length[0] / 3.0 * _Ay[0] *
          (_local_rot_accel_0(0) + _local_rot_accel_1(0) / 2.0 +
           _eta[0] * (1.0 + _alpha) * (_local_rot_vel_0(0) + _local_rot_vel_1(0) / 2.0) -
           _alpha * _eta[0] * (_local_rot_vel_old_0(0) + _local_rot_vel_old_1(0) / 2.0));
      _local_force[1](2) +=
          _density[0] * _original_length[0] / 3.0 * _Ay[0] *
          (_local_rot_accel_1(0) + _local_rot_accel_0(0) / 2.0 +
           _eta[0] * (1.0 + _alpha) * (_local_rot_vel_1(0) + _local_rot_vel_0(0) / 2.0) -
           _alpha * _eta[0] * (_local_rot_vel_old_1(0) + _local_rot_vel_old_0(0) / 2.0));
    }
    else
    {
      _local_moment[0](0) += _density[0] * _original_length[0] / 3.0 *
                             (-_Az[0] * (_local_accel_0(1) + _local_accel_1(1) / 2.0) +
                              _Ay[0] * (_local_accel_0(1) + _local_accel_1(1) / 2.0));
      _local_moment[1](0) += _density[0] * _original_length[0] / 3.0 *
                             (-_Az[0] * (_local_accel_1(1) + _local_accel_0(1) / 2.0) +
                              _Ay[0] * (_local_accel_1(1) + _local_accel_0(1) / 2.0));

      _local_moment[0](1) += _density[0] * _original_length[0] / 3.0 * _Az[0] *
                             (_local_accel_0(0) + _local_accel_1(0) / 2.0);
      _local_moment[1](1) += _density[0] * _original_length[0] / 3.0 * _Az[0] *
                             (_local_accel_1(0) + _local_accel_0(0) / 2.0);

      _local_moment[0](2) += -_density[0] * _original_length[0] / 3.0 * _Ay[0] *
                             (_local_accel_0(0) + _local_accel_1(0) / 2.0);
      _local_moment[1](2) += -_density[0] * _original_length[0] / 3.0 * _Ay[0] *
                             (_local_accel_1(0) + _local_accel_0(0) / 2.0);
    }

    // Global force and moments
    if (_component < 3)
    {
      _global_force_0 = _original_local_config[0] * _local_force[0];
      _global_force_1 = _original_local_config[0] * _local_force[1];
      _local_re(0) = _global_force_0(_component);
      _local_re(1) = _global_force_1(_component);
    }
    else
    {
      _global_moment_0 = _original_local_config[0] * _local_moment[0];
      _global_moment_1 = _original_local_config[0] * _local_moment[1];
      _local_re(0) = _global_moment_0(_component - 3);
      _local_re(1) = _global_moment_1(_component - 3);
    }
  }

  accumulateTaggedLocalResidual();

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i = 0; i < _save_in.size(); ++i)
      _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }
}

void
InertialForceBeam::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());

  mooseAssert(_beta > 0.0, "InertialForceBeam: Beta parameter should be positive.");

  Real factor = 0.0;
  if (_has_beta)
    factor = 1.0 / (_beta * _dt * _dt) + _eta[0] * (1.0 + _alpha) * _gamma / _beta / _dt;
  else
    factor = (*_du_dotdot_du)[0] + _eta[0] * (1.0 + _alpha) * (*_du_dot_du)[0];

  for (unsigned int i = 0; i < _test.size(); ++i)
  {
    for (unsigned int j = 0; j < _phi.size(); ++j)
    {
      if (_component < 3)
        _local_ke(i, j) = (i == j ? 1.0 / 3.0 : 1.0 / 6.0) * _density[0] * _area[0] *
                          _original_length[0] * factor;
      else if (_component > 2)
      {
        RankTwoTensor I;
        if (_has_Ix)
          I(0, 0) = _Ix[0];
        else
          I(0, 0) = _Iy[0] + _Iz[0];
        I(1, 1) = _Iz[0];
        I(2, 2) = _Iy[0];

        // conversion from local config to global coordinate system
        RankTwoTensor Ig = _original_local_config[0].transpose() * I * _original_local_config[0];

        _local_ke(i, j) = (i == j ? 1.0 / 3.0 : 1.0 / 6.0) * _density[0] *
                          Ig(_component - 3, _component - 3) * _original_length[0] * factor;
      }
    }
  }

  accumulateTaggedLocalMatrix();

  if (_has_diag_save_in)
  {
    unsigned int rows = _local_ke.m();
    DenseVector<Number> diag(rows);
    for (unsigned int i = 0; i < rows; ++i)
      diag(i) = _local_ke(i, i);

    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i = 0; i < _diag_save_in.size(); ++i)
      _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
  }
}

void
InertialForceBeam::computeOffDiagJacobian(const unsigned int jvar_num)
{
  mooseAssert(_beta > 0.0, "InertialForceBeam: Beta parameter should be positive.");

  Real factor = 0.0;
  if (_has_beta)
    factor = 1.0 / (_beta * _dt * _dt) + _eta[0] * (1.0 + _alpha) * _gamma / _beta / _dt;
  else
    factor = (*_du_dotdot_du)[0] + _eta[0] * (1.0 + _alpha) * (*_du_dot_du)[0];

  if (jvar_num == _var.number())
    computeJacobian();
  else
  {
    unsigned int coupled_component = 0;
    bool disp_coupled = false;
    bool rot_coupled = false;

    for (unsigned int i = 0; i < _ndisp; ++i)
    {
      if (jvar_num == _disp_num[i] && _component > 2)
      {
        coupled_component = i;
        disp_coupled = true;
        break;
      }
    }

    for (unsigned int i = 0; i < _nrot; ++i)
    {
      if (jvar_num == _rot_num[i])
      {
        coupled_component = i + 3;
        rot_coupled = true;
        break;
      }
    }

    prepareMatrixTag(_assembly, _var.number(), jvar_num);

    if (disp_coupled || rot_coupled)
    {
      for (unsigned int i = 0; i < _test.size(); ++i)
      {
        for (unsigned int j = 0; j < _phi.size(); ++j)
        {
          if (_component < 3 && coupled_component > 2)
          {
            RankTwoTensor A;
            A(0, 1) = _Az[0];
            A(0, 2) = -_Ay[0];
            A(1, 0) = -_Az[0];
            A(2, 0) = _Ay[0];

            // conversion from local config to global coordinate system
            const RankTwoTensor Ag =
                _original_local_config[0].transpose() * A * _original_local_config[0];

            _local_ke(i, j) += (i == j ? 1.0 / 3.0 : 1.0 / 6.0) * _density[0] *
                               Ag(_component, coupled_component - 3) * _original_length[0] * factor;
          }
          else if (_component > 2 && coupled_component < 3)
          {
            RankTwoTensor A;
            A(0, 1) = -_Az[0];
            A(0, 2) = _Ay[0];
            A(1, 0) = _Az[0];
            A(2, 0) = -_Ay[0];

            // conversion from local config to global coordinate system
            const RankTwoTensor Ag =
                _original_local_config[0].transpose() * A * _original_local_config[0];

            _local_ke(i, j) += (i == j ? 1.0 / 3.0 : 1.0 / 6.0) * _density[0] *
                               Ag(_component - 3, coupled_component) * _original_length[0] * factor;
          }
          else if (_component > 2 && coupled_component > 2)
          {
            RankTwoTensor I;
            if (_has_Ix)
              I(0, 0) = _Ix[0];
            else
              I(0, 0) = _Iy[0] + _Iz[0];
            I(1, 1) = _Iz[0];
            I(2, 2) = _Iy[0];

            // conversion from local config to global coordinate system
            const RankTwoTensor Ig =
                _original_local_config[0].transpose() * I * _original_local_config[0];

            _local_ke(i, j) += (i == j ? 1.0 / 3.0 : 1.0 / 6.0) * _density[0] *
                               Ig(_component - 3, coupled_component - 3) * _original_length[0] *
                               factor;
          }
        }
      }
    }

    accumulateTaggedLocalMatrix();
  }
}
