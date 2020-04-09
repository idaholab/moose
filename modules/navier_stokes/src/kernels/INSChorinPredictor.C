//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSChorinPredictor.h"
#include "MooseMesh.h"

registerMooseObject("NavierStokesApp", INSChorinPredictor);

InputParameters
INSChorinPredictor::validParams()
{
  InputParameters params = Kernel::validParams();

  params.addClassDescription("This class computes the 'Chorin' Predictor equation in "
                             "fully-discrete (both time and space) form.");
  // Coupled variables
  params.addRequiredCoupledVar("u", "x-velocity");
  params.addCoupledVar("v", "y-velocity"); // only required in 2D and 3D
  params.addCoupledVar("w", "z-velocity"); // only required in 3D

  // Make star also be required, even though we might not use it?
  params.addRequiredCoupledVar("u_star", "star x-velocity");
  params.addCoupledVar("v_star", "star y-velocity"); // only required in 2D and 3D
  params.addCoupledVar("w_star", "star z-velocity"); // only required in 3D

  // Required parameters
  params.addRequiredRangeCheckedParam<unsigned>(
      "component",
      "component>=0 & component<=2",
      "0,1,2 depending on if we are solving the x,y,z component of the Predictor equation");
  MooseEnum predictor_type("OLD NEW STAR");
  params.addRequiredParam<MooseEnum>(
      "predictor_type",
      predictor_type,
      "One of: OLD, NEW, STAR.  Indicates which velocity to use in the predictor.");

  // Optional parameters
  params.addParam<MaterialPropertyName>("mu_name", "mu", "The name of the dynamic viscosity");
  params.addParam<MaterialPropertyName>("rho_name", "rho", "The name of the density");

  return params;
}

INSChorinPredictor::INSChorinPredictor(const InputParameters & parameters)
  : Kernel(parameters),

    // Current velocities
    _u_vel(coupledValue("u")),
    _v_vel(_mesh.dimension() >= 2 ? coupledValue("v") : _zero),
    _w_vel(_mesh.dimension() == 3 ? coupledValue("w") : _zero),

    // Old velocities
    _u_vel_old(coupledValueOld("u")),
    _v_vel_old(_mesh.dimension() >= 2 ? coupledValueOld("v") : _zero),
    _w_vel_old(_mesh.dimension() == 3 ? coupledValueOld("w") : _zero),

    // Star velocities
    _u_vel_star(coupledValue("u_star")),
    _v_vel_star(_mesh.dimension() >= 2 ? coupledValue("v_star") : _zero),
    _w_vel_star(_mesh.dimension() == 3 ? coupledValue("w_star") : _zero),

    // Velocity Gradients
    _grad_u_vel(coupledGradient("u")),
    _grad_v_vel(_mesh.dimension() >= 2 ? coupledGradient("v") : _grad_zero),
    _grad_w_vel(_mesh.dimension() == 3 ? coupledGradient("w") : _grad_zero),

    // Old Velocity Gradients
    _grad_u_vel_old(coupledGradientOld("u")),
    _grad_v_vel_old(_mesh.dimension() >= 2 ? coupledGradientOld("v") : _grad_zero),
    _grad_w_vel_old(_mesh.dimension() == 3 ? coupledGradientOld("w") : _grad_zero),

    // Star Velocity Gradients
    _grad_u_vel_star(coupledGradient("u_star")),
    _grad_v_vel_star(_mesh.dimension() >= 2 ? coupledGradient("v_star") : _grad_zero),
    _grad_w_vel_star(_mesh.dimension() == 3 ? coupledGradient("w_star") : _grad_zero),

    // Variable numberings
    _u_vel_var_number(coupled("u")),
    _v_vel_var_number(_mesh.dimension() >= 2 ? coupled("v") : libMesh::invalid_uint),
    _w_vel_var_number(_mesh.dimension() == 3 ? coupled("w") : libMesh::invalid_uint),

    // Star velocity numberings
    _u_vel_star_var_number(coupled("u_star")),
    _v_vel_star_var_number(_mesh.dimension() >= 2 ? coupled("v_star") : libMesh::invalid_uint),
    _w_vel_star_var_number(_mesh.dimension() == 3 ? coupled("w_star") : libMesh::invalid_uint),

    // Required parameters
    _component(getParam<unsigned>("component")),
    _predictor_enum(getParam<MooseEnum>("predictor_type")),

    // Material properties
    _mu(getMaterialProperty<Real>("mu_name")),
    _rho(getMaterialProperty<Real>("rho_name"))
{
}

Real
INSChorinPredictor::computeQpResidual()
{
  // Vector object for test function
  RealVectorValue test;
  test(_component) = _test[_i][_qp];

  // Tensor object for test function gradient
  RealTensorValue grad_test;
  for (unsigned k = 0; k < 3; ++k)
    grad_test(_component, k) = _grad_test[_i][_qp](k);

  // Decide what velocity vector, gradient to use:
  RealVectorValue U;
  RealTensorValue grad_U;

  switch (_predictor_enum)
  {
    case OLD:
    {
      U = RealVectorValue(_u_vel_old[_qp], _v_vel_old[_qp], _w_vel_old[_qp]);
      grad_U = RealTensorValue(_grad_u_vel_old[_qp], _grad_v_vel_old[_qp], _grad_w_vel_old[_qp]);
      break;
    }
    case NEW:
    {
      U = RealVectorValue(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
      grad_U = RealTensorValue(_grad_u_vel[_qp], _grad_v_vel[_qp], _grad_w_vel[_qp]);
      break;
    }
    case STAR:
    {
      // Note: Donea and Huerta's book says you are supposed to use "star" velocity to make Chorin
      // implicit, not U^{n+1}.
      U = RealVectorValue(_u_vel_star[_qp], _v_vel_star[_qp], _w_vel_star[_qp]);
      grad_U = RealTensorValue(_grad_u_vel_star[_qp], _grad_v_vel_star[_qp], _grad_w_vel_star[_qp]);
      break;
    }
    default:
      mooseError("Unrecognized Chorin predictor type requested.");
  }

  //
  // Compute the different parts
  //

  // Note: _u is the component'th entry of "u_star" in Chorin's method.
  RealVectorValue U_old(_u_vel_old[_qp], _v_vel_old[_qp], _w_vel_old[_qp]);
  Real symmetric_part = (_u[_qp] - U_old(_component)) * _test[_i][_qp];

  // Convective part.  Remember to multiply by _dt!
  Real convective_part = _dt * (grad_U * U) * test;

  // Viscous part - we are using the Laplacian form here for simplicity.
  // Remember to multiply by _dt!
  Real viscous_part = _dt * (_mu[_qp] / _rho[_qp]) * grad_U.contract(grad_test);

  return symmetric_part + convective_part + viscous_part;
}

Real
INSChorinPredictor::computeQpJacobian()
{
  // The mass matrix part is always there.
  Real mass_part = _phi[_j][_qp] * _test[_i][_qp];

  // The on-diagonal Jacobian contribution depends on whether the predictor uses the
  // 'new' or 'star' velocity.
  Real other_part = 0.;
  switch (_predictor_enum)
  {
    case OLD:
    case NEW:
      break;
    case STAR:
    {
      RealVectorValue U_star(_u_vel_star[_qp], _v_vel_star[_qp], _w_vel_star[_qp]);
      Real convective_part =
          _dt * ((U_star * _grad_phi[_j][_qp]) + _phi[_j][_qp] * _grad_u[_qp](_component)) *
          _test[_i][_qp];
      Real viscous_part =
          _dt * ((_mu[_qp] / _rho[_qp]) * (_grad_phi[_j][_qp] * _grad_test[_i][_qp]));
      other_part = convective_part + viscous_part;
      break;
    }
    default:
      mooseError("Unrecognized Chorin predictor type requested.");
  }

  return mass_part + other_part;
}

Real
INSChorinPredictor::computeQpOffDiagJacobian(unsigned jvar)
{
  switch (_predictor_enum)
  {
    case OLD:
    {
      return 0.;
    }

    case NEW:
    {
      if ((jvar == _u_vel_var_number) || (jvar == _v_vel_var_number) || (jvar == _w_vel_var_number))
      {
        // Derivative of grad_U wrt the velocity component
        RealTensorValue dgrad_U;

        // Initialize to invalid value, then determine correct value.
        unsigned vel_index = 99;

        // Map jvar into the indices (0,1,2)
        if (jvar == _u_vel_var_number)
          vel_index = 0;

        else if (jvar == _v_vel_var_number)
          vel_index = 1;

        else if (jvar == _w_vel_var_number)
          vel_index = 2;

        // Fill in the vel_index'th row of dgrad_U with _grad_phi[_j][_qp]
        for (unsigned k = 0; k < 3; ++k)
          dgrad_U(vel_index, k) = _grad_phi[_j][_qp](k);

        // Vector object for test function
        RealVectorValue test;
        test(_component) = _test[_i][_qp];

        // Vector object for U
        RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

        // Tensor object for test function gradient
        RealTensorValue grad_test;
        for (unsigned k = 0; k < 3; ++k)
          grad_test(_component, k) = _grad_test[_i][_qp](k);

        // Compute the convective part
        RealVectorValue convective_jac =
            _phi[_j][_qp] * RealVectorValue(_grad_u_vel[_qp](vel_index),
                                            _grad_v_vel[_qp](vel_index),
                                            _grad_w_vel[_qp](vel_index));

        // Extra contribution in vel_index component
        convective_jac(vel_index) += U * _grad_phi[_j][_qp];

        // Be sure to scale by _dt!
        Real convective_part = _dt * (convective_jac * test);

        // Compute the viscous part, be sure to scale by _dt.  Note: the contracted
        // value should be zero unless vel_index and _component match.
        Real viscous_part = _dt * (_mu[_qp] / _rho[_qp]) * dgrad_U.contract(grad_test);

        // Return the result
        return convective_part + viscous_part;
      }
      else
        return 0;
    }

    case STAR:
    {
      if (jvar == _u_vel_star_var_number)
      {
        return _dt * _phi[_j][_qp] * _grad_u[_qp](0) * _test[_i][_qp];
      }

      else if (jvar == _v_vel_star_var_number)
      {
        return _dt * _phi[_j][_qp] * _grad_u[_qp](1) * _test[_i][_qp];
      }

      else if (jvar == _w_vel_star_var_number)
      {
        return _dt * _phi[_j][_qp] * _grad_u[_qp](2) * _test[_i][_qp];
      }

      else
        return 0;
    }

    default:
      mooseError("Unrecognized Chorin predictor type requested.");
  }
}
