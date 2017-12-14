/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "NodalInertialTorque.h"
#include "MooseVariable.h"
#include "AuxiliarySystem.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<NodalInertialTorque>()
{
  InputParameters params = validParams<NodalKernel>();
  params.addRequiredCoupledVar("rotations", "rotational displacement variables");
  params.addRequiredCoupledVar("rot_velocities", "rotational velocity variables");
  params.addRequiredCoupledVar("rot_accelerations", "rotational acceleration variables");
  params.addRequiredParam<Real>("beta", "beta parameter for Newmark Time integration");
  params.addRequiredParam<Real>("gamma", "gamma parameter for Newmark Time integration");
  params.addParam<Real>("eta",
                        0.0,
                        "Name of material property or a constant real "
                        "number defining the eta parameter for the "
                        "Rayleigh damping.");
  params.addParam<Real>("alpha",
                        0,
                        "alpha parameter for mass dependent numerical damping induced "
                        "by HHT time integration scheme");
  params.addRequiredParam<Real>("Ixx", "Moment of inertia in the x direction.");
  params.addRequiredParam<Real>("Iyy", "Moment of inertia in the y direction.");
  params.addRequiredParam<Real>("Izz", "Moment of inertia in the z direction.");
  params.addParam<Real>("Ixy", 0.0, "Moment of inertia in the xy direction.");
  params.addParam<Real>("Ixz", 0.0, "Moment of inertia in the xz direction.");
  params.addParam<Real>("Iyz", 0.0, "Moment of inertia in the yz direction.");
  params.addParam<RealGradient>(
      "x_orientation", "Unit vector along the x direction if different from global x direction.");
  params.addParam<RealGradient>(
      "y_orientation", "Unit vector along the y direction if different from global y direction.");
  params.addRequiredParam<unsigned int>("component",
                                        "An integer corresponding to the direction "
                                        "the variable this kernel acts in. (0 for rot_x, "
                                        "1 for rot_y, and 2 for rot_z).");
  return params;
}

NodalInertialTorque::NodalInertialTorque(const InputParameters & parameters)
  : NodalKernel(parameters),
    _aux_sys(_fe_problem.getAuxiliarySystem()),
    _nrot(coupledComponents("rotations")),
    _rot(3),
    _rot_old(3),
    _rot_vel_num(3),
    _rot_accel_num(3),
    _rot_variables(3),
    _beta(getParam<Real>("beta")),
    _gamma(getParam<Real>("gamma")),
    _eta(getParam<Real>("eta")),
    _alpha(getParam<Real>("alpha")),
    _component(getParam<unsigned int>("component"))
{
  if (coupledComponents("rot_velocities") != _nrot ||
      coupledComponents("rot_accelerations") != _nrot)
    mooseError("rot_velocities and rot_accelerations should be same size as rotations.");

  for (unsigned int i = 0; i < _nrot; ++i)
  {
    MooseVariable * rot_var = getVar("rotations", i);
    MooseVariable * rot_vel_var = getVar("rot_velocities", i);
    MooseVariable * rot_accel_var = getVar("rot_accelerations", i);

    _rot[i] = &rot_var->nodalSln();
    _rot_old[i] = &rot_var->nodalSlnOld();

    _rot_vel_num[i] = rot_vel_var->number();
    _rot_accel_num[i] = rot_accel_var->number();

    _rot_variables[i] = coupled("rotations", i);
  }

  for (unsigned int i = _nrot; i < 3; ++i)
  {
    _rot[i] = &_zero;
    _rot_old[i] = &_zero;
  }

  // Store inertia values in inertia tensor
  _inertia.zero();
  _inertia(0, 0) = getParam<Real>("Ixx");
  _inertia(0, 1) = getParam<Real>("Ixy");
  _inertia(0, 2) = getParam<Real>("Ixz");
  _inertia(1, 0) = _inertia(0, 1);
  _inertia(1, 1) = getParam<Real>("Iyy");
  _inertia(1, 2) = getParam<Real>("Iyz");
  _inertia(2, 0) = _inertia(0, 2);
  _inertia(2, 1) = _inertia(1, 2);
  _inertia(2, 2) = getParam<Real>("Izz");

  // Check for positive definiteness of matrix using Slyvester's criterion.
  Real det1 = _inertia(0, 0);
  Real det2 = _inertia(0, 0) * _inertia(1, 1) - _inertia(0, 1) * _inertia(1, 0);
  Real det3 = _inertia.det();

  if (det1 < 1e-6 || det2 < 1e-6 || det3 < 1e-6)
    mooseError("The moment of inertia tensor should be positive definite.");
  if (isParamValid("x_orientation") && isParamValid("y_orientation"))
  {
    RealGradient x_orientation = getParam<RealGradient>("x_orientation");
    RealGradient y_orientation = getParam<RealGradient>("y_orientation");

    if (std::abs(x_orientation.norm() - 1.0) > 1e-6 || std::abs(y_orientation.norm() - 1.0) > 1e-6)
      mooseError("x_orientation and y_orientation must be unit vectors.");

    Real sum = x_orientation(0) * y_orientation(0) + x_orientation(1) * y_orientation(1) +
               x_orientation(2) * y_orientation(2);

    if (abs(sum) > 1e-4)
      mooseError("x_orientation and y_orientation should be perpendicular to each other.");

    // Calculate z orientation as a cross product of the x and y orientations
    RealGradient z_orientation(3, 0.0);
    z_orientation(0) = (x_orientation(1) * y_orientation(2) - x_orientation(2) * y_orientation(1));
    z_orientation(1) = (x_orientation(2) * y_orientation(0) - x_orientation(0) * y_orientation(2));
    z_orientation(2) = (x_orientation(0) * y_orientation(1) - x_orientation(1) * y_orientation(0));

    // Rotation matrix from global to original beam local configuration
    RankTwoTensor orientation;
    orientation(0, 0) = x_orientation(0);
    orientation(0, 1) = x_orientation(1);
    orientation(0, 2) = x_orientation(2);
    orientation(1, 0) = y_orientation(0);
    orientation(1, 1) = y_orientation(1);
    orientation(1, 2) = y_orientation(2);
    orientation(2, 0) = z_orientation(0);
    orientation(2, 1) = z_orientation(1);
    orientation(2, 2) = z_orientation(2);

    RankTwoTensor global_inertia = orientation.transpose() * _inertia * orientation;
    _inertia = global_inertia;
  }
  else if ((isParamValid("x_orientation") && !isParamValid("y_orientation")) ||
           (!isParamValid("x_orientation") && isParamValid("y_orientation")))
    mooseError("Both x_orientation and y_orientation should be provided if x_orientation or "
               "y_orientation is different from global x or y direction, respectively.");
}

Real
NodalInertialTorque::computeQpResidual()
{
  if (_dt == 0)
    return 0;
  else
  {
    const NumericVector<Number> & aux_sol_old = _aux_sys.solutionOld();

    std::vector<Real> rot_accel(3, 0.0);
    std::vector<Real> rot_vel(3, 0.0);
    std::vector<Real> rot_vel_old(3, 0.0);
    for (unsigned int i = 0; i < _nrot; ++i)
    {
      rot_vel_old[i] =
          aux_sol_old(_current_node->dof_number(_aux_sys.number(), _rot_vel_num[i], 0));
      Real rot_accel_old =
          aux_sol_old(_current_node->dof_number(_aux_sys.number(), _rot_accel_num[i], 0));

      rot_accel[i] = 1. / _beta * ((((*_rot[i])[_qp] - (*_rot_old[i])[_qp]) / (_dt * _dt)) -
                                   rot_vel_old[i] / _dt - rot_accel_old * (0.5 - _beta));
      rot_vel[i] =
          rot_vel_old[i] + (_dt * (1 - _gamma)) * rot_accel_old + _gamma * _dt * rot_accel[i];
    }

    Real res = 0.0;
    for (unsigned int i = 0; i < _nrot; ++i)
      res += _inertia(_component, i) *
             (rot_accel[i] + rot_vel[i] * _eta * (1.0 + _alpha) - _alpha * _eta * rot_vel_old[i]);

    return res;
  }
}

Real
NodalInertialTorque::computeQpJacobian()
{
  if (_dt == 0)
    return 0.0;
  else
    return _inertia(_component, _component) / (_beta * _dt * _dt) +
           _eta * (1 + _alpha) * _inertia(_component, _component) * _gamma / _beta / _dt;
}

Real
NodalInertialTorque::computeQpOffDiagJacobian(unsigned int jvar)
{
  unsigned int coupled_component = 0;
  bool rot_coupled = false;

  for (unsigned int i = 0; i < _nrot; ++i)
  {
    if (jvar == _rot_variables[i])
    {
      coupled_component = i;
      rot_coupled = true;
      break;
    }
  }

  if (_dt == 0)
    return 0.0;
  else if (rot_coupled)
    return _inertia(_component, coupled_component) / (_beta * _dt * _dt) +
           _eta * (1 + _alpha) * _inertia(_component, coupled_component) * _gamma / _beta / _dt;
  else
    return 0.0;
}
