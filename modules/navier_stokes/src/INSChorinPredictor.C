#include "INSChorinPredictor.h"

template<>
InputParameters validParams<INSChorinPredictor>()
{
  InputParameters params = validParams<Kernel>();

  // Coupled variables
  params.addRequiredCoupledVar("u", "x-velocity");
  params.addCoupledVar("v", "y-velocity"); // only required in 2D and 3D
  params.addCoupledVar("w", "z-velocity"); // only required in 3D

  // Required parameters
  params.addRequiredParam<Real>("mu", "dynamic viscosity");
  params.addRequiredParam<Real>("rho", "density");
  params.addRequiredParam<unsigned>("component", "0,1,2 depending on if we are solving the x,y,z component of the Predictor equation");
  params.addRequiredParam<bool>("chorin_implicit", "Flag which controls the time level the convection and diffusion terms are evaluated at.");

  return params;
}



INSChorinPredictor::INSChorinPredictor(const std::string & name, InputParameters parameters) :
  Kernel(name, parameters),

  // Current velocities
  _u_vel(coupledValue("u")),
  _v_vel(_dim >= 2 ? coupledValue("v") : _zero),
  _w_vel(_dim == 3 ? coupledValue("w") : _zero),

  // Old velocities
  _u_vel_old(coupledValueOld("u")),
  _v_vel_old(_dim >= 2 ? coupledValueOld("v") : _zero),
  _w_vel_old(_dim == 3 ? coupledValueOld("w") : _zero),

  // Current Gradients
  _grad_u_vel(coupledGradient("u")),
  _grad_v_vel(_dim >= 2 ? coupledGradient("v") : _grad_zero),
  _grad_w_vel(_dim == 3 ? coupledGradient("w") : _grad_zero),

  // Old Gradients
  _grad_u_vel_old(coupledGradientOld("u")),
  _grad_v_vel_old(_dim >= 2 ? coupledGradientOld("v") : _grad_zero),
  _grad_w_vel_old(_dim == 3 ? coupledGradientOld("w") : _grad_zero),

  // Variable numberings
  _u_vel_var_number(coupled("u")),
  _v_vel_var_number(_dim >= 2 ? coupled("v") : libMesh::invalid_uint),
  _w_vel_var_number(_dim == 3 ? coupled("w") : libMesh::invalid_uint),

  // Required parameters
  _mu(getParam<Real>("mu")),
  _rho(getParam<Real>("rho")),
  _component(getParam<unsigned>("component")),
  _chorin_implicit(getParam<bool>("chorin_implicit"))
{
}



Real INSChorinPredictor::computeQpResidual()
{
  // Vector object for U_old
  RealVectorValue U_old(_u_vel_old[_qp], _v_vel_old[_qp], _w_vel_old[_qp]);

  // Vector object for U
  RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // Tensor object for "grad U" = d(u_i)/d(x_j)
  RealTensorValue grad_U(_grad_u_vel[_qp],
                         _grad_v_vel[_qp],
                         _grad_w_vel[_qp]);

  // Tensor object for "grad U_old"
  RealTensorValue grad_U_old(_grad_u_vel_old[_qp],
                             _grad_v_vel_old[_qp],
                             _grad_w_vel_old[_qp]);

  // Vector object for test function
  RealVectorValue test;
  test(_component) = _test[_i][_qp];

  // Tensor object for test function gradient
  RealTensorValue grad_test;
  for (unsigned k=0; k<3; ++k)
    grad_test(_component, k) = _grad_test[_i][_qp](k);

  //
  // Compute the different parts
  //

  // Note: _u is the component'th entry of "u_star" in Chorin's method
  Real symmetric_part = (_u[_qp] - U_old(_component)) * _test[_i][_qp];

  // Convective part - may be evaluated at old or new time.  Remember to multiply by _dt!
  Real convective_part = _chorin_implicit ? (_dt * (grad_U * U) * test) : (_dt * (grad_U_old * U_old) * test);

  // Viscous part - we are using the Laplacian form here for simplicity... this
  // can also be evaluated at old or new time.  Remember to multiply by _dt!
  Real viscous_part = _chorin_implicit ? (_dt * (_mu/_rho) * grad_U.contract(grad_test)) : (_dt * (_mu/_rho) * grad_U_old.contract(grad_test));

  return symmetric_part + convective_part + viscous_part;
}




Real INSChorinPredictor::computeQpJacobian()
{
  // The on-diagonal Jacobian contribution is just the mass matrix entry.
  return _phi[_j][_qp] * _test[_i][_qp];
}




Real INSChorinPredictor::computeQpOffDiagJacobian(unsigned jvar)
{
  // Implicit case
  if (_chorin_implicit)
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
      for (unsigned k=0; k<3; ++k)
        dgrad_U(vel_index,k) = _grad_phi[_j][_qp](k);

      // Vector object for test function
      RealVectorValue test;
      test(_component) = _test[_i][_qp];

      // Vector object for U
      RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

      // Tensor object for test function gradient
      RealTensorValue grad_test;
      for (unsigned k=0; k<3; ++k)
        grad_test(_component, k) = _grad_test[_i][_qp](k);

      // Compute the convective part
      RealVectorValue convective_jac = _phi[_j][_qp] * RealVectorValue(_grad_u_vel[_qp](vel_index),
                                                                       _grad_v_vel[_qp](vel_index),
                                                                       _grad_w_vel[_qp](vel_index));

      // Extra contribution in vel_index component
      convective_jac(vel_index) += U*_grad_phi[_j][_qp];

      // Be sure to scale by _dt!
      Real convective_part = _dt * (convective_jac * test);

      // Compute the viscous part, be sure to scale by _dt.  Note: the contracted
      // value should be zero unless vel_index and _component match.
      Real viscous_part = _dt * (_mu/_rho) * dgrad_U.contract(grad_test);

      // Return the result
      return convective_part + viscous_part;
    }
    else
      return 0;
  }


  // Explicit case
  else
  {
    // If we're using "old" values on the right hand side, the off-diagonal
    // Jacobians are zero.
    return 0;
  }
}
