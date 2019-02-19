#include "OneDStabilizationBase.h"

template <>
InputParameters
validParams<OneDStabilizationBase>()
{
  // Initialize the params object from the base class
  InputParameters params = validParams<Kernel>();

  // Required coupled variables (so we can get variable numbers)
  params.addRequiredCoupledVar("arhoA", "alpha*rho*A");
  params.addRequiredCoupledVar("arhouA", "alpha*rho*u*A");
  params.addRequiredCoupledVar("arhoEA", "alpha*rho*E*A");

  // The velocity aux's gradient is required if we need to do Lapidus
  params.addRequiredCoupledVar("vel", "x-velocity");

  params.addRequiredParam<MaterialPropertyName>("direction",
                                                "The direction of the pipe material property");
  params.addRequiredParam<MaterialPropertyName>("SUPG_delta",
                                                "Stabilization delta material property");
  params.addRequiredParam<MaterialPropertyName>("SUPG_R",
                                                "Stabilization residual material property");
  params.addRequiredParam<MaterialPropertyName>("SUPG_A", "Stabilization matrix material property");
  params.addRequiredParam<MaterialPropertyName>("SUPG_y",
                                                "Stabilization columns material property");
  params.addRequiredParam<MaterialPropertyName>("SUPG_dUdx",
                                                "Stabilization gradient material property");
  params.addRequiredParam<MaterialPropertyName>("SUPG_dAdU",
                                                "Stabilization derivative material property");

  return params;
}

OneDStabilizationBase::OneDStabilizationBase(const InputParameters & parameters)
  : Kernel(parameters),
    _delta_matprop(getMaterialProperty<Real>("SUPG_delta")),
    _R(getMaterialProperty<RealVectorValue>("SUPG_R")),
    _A(getMaterialProperty<RealTensorValue>("SUPG_A")),
    _y(getMaterialProperty<std::vector<RealVectorValue>>("SUPG_y")),
    _dUdx(getMaterialProperty<RealVectorValue>("SUPG_dUdx")),
    _dA(getMaterialProperty<std::vector<RealTensorValue>>("SUPG_dAdU")),
    _arhoA_var_number(coupled("arhoA")),
    _arhouA_var_number(coupled("arhouA")),
    _arhoEA_var_number(coupled("arhoEA")),
    _d_arhodot_du(coupledDotDu("arhoA")),
    _d_arhoudot_du(coupledDotDu("arhouA")),
    _d_arhoEdot_du(coupledDotDu("arhoEA")),
    _grad_vel(coupledGradient("vel")),
    _dir(getMaterialProperty<RealVectorValue>("direction"))
{
}

Real
OneDStabilizationBase::supg_residual_contribution(unsigned row)
{
  //  // Are we using Trapezoidal rule quadrature?
  //  if (_assembly.qRule()->type() == QTRAP)
  //  {
  //    // Is _current_elem on the boundary and are we on that boundary's qp?
  //    // If so, then return 0 for the stabilization term
  //    if (((_current_elem->neighbor(0) == nullptr) && (_qp==0)) ||
  //        ((_current_elem->neighbor(1) == nullptr) && (_qp==1)))
  //    {
  //      // Try returning 0 - this was actually worse than doing SUPG
  //      return 0.;
  //
  //      // // Try a Lapidus (isotropic) artificial diffusion on the boundary.
  //      // // To do this, we need to couple to the velocity gradient.
  //      // Real h = _current_elem->hmax();
  //      // Real c_Lap = 1.0;
  //      // Real coef = c_Lap * h * h * std::abs(_grad_vel[_qp](0));
  //      //
  //      // Real ret = coef * _grad_u[_qp](0) * _grad_test[_i][_qp](0);
  //      // // _console << "Returning Lapidus value " << ret << std::endl;
  //      // return ret;
  //    }
  //  }

  Real sum = 0.;
  for (unsigned ell = 0; ell < 3; ++ell)
  {
    Real dot = 0.;
    // Note: if _A is implemented as a vector<RealVectorValue> of *rows* of A, we
    // could eliminate this for-loop.
    for (unsigned i = 0; i < 3; ++i)
      dot += _A[_qp](row, i) * _y[_qp][ell](i); // a_k * y_{ell}
    // (a_k * y_{ell}) * R_{ell}
    sum += dot * _R[_qp](ell);
  }

  // Return sum scaled by d(phi)/dx
  return sum * _grad_test[_i][_qp] * _dir[_qp];

  // // Do "Lapidus" stabilization instead of SUPG
  // Real h = _current_elem->hmax();
  // Real c_Lap = 2.0;
  // Real coef = c_Lap * h * h * std::abs(_grad_vel[_qp](0));
  //
  // Real ret = coef * _grad_u[_qp](0) * _grad_test[_i][_qp](0);
  // // _console << "Returning Lapidus value " << ret << std::endl;
  // return ret;
}

Real
OneDStabilizationBase::supg_jacobian_contribution(unsigned k, unsigned m)
{
  // Makes accessing the "time derivative derivatives" (not a typo!) easier
  Real d_udot_du[3] = {_d_arhodot_du[_qp], _d_arhoudot_du[_qp], _d_arhoEdot_du[_qp]};

  // Matrix-vector product of the derivative of A wrt the mth conserved variable, times the gradient
  // of the conserved variables vector.
  RealVectorValue Am_dUdx = _dA[_qp][m] * _dUdx[_qp];

  // Debugging:
  // if (Am_dUdx.size_sq() > 1.)
  //  _console << "Am_dUdx = " << Am_dUdx << std::endl;

  // Eventual return value
  Real ret = 0.;

  for (unsigned ell = 0; ell < 3; ++ell)
  {
    Real dot = 0., time_part = 0.;

    for (unsigned i = 0; i < 3; ++i)
      dot += _A[_qp](k, i) * _y[_qp][ell](i); // a_k * y_{ell}

    // If ell==m, then add the time part
    if (ell == m)
      time_part = d_udot_du[m];

    // Multiply (a_k * y_{ell}) by A(ell,m), an entry from the
    // mth column of the advective flux matrix, and accumulate the result.
    ret += dot * (time_part * _phi[_j][_qp] + _A[_qp](ell, m) * _grad_phi[_j][_qp] * _dir[_qp] +
                  Am_dUdx(ell) * _phi[_j][_qp]);
  }

  // The Jacobian contribution due to the SUPG terms
  return ret * _grad_test[_i][_qp] * _dir[_qp];
}

unsigned
OneDStabilizationBase::map_moose_var(unsigned jvar)
{
  if (jvar == _arhoA_var_number)
    return 0;
  else if (jvar == _arhouA_var_number)
    return 1;
  else if (jvar == _arhoEA_var_number)
    return 2;
  else
  {
    // If you are debugging, you might try uncommenting this print
    // statement and seeing if it clears up anything for you...
    // Moose::err << "Warning: invalid jvar = "
    //           << jvar
    //           << ", _arho_var_number = "
    //           << _arho_var_number
    //           << ", _arhou_var_number = "
    //           << _arhou_var_number
    //           << ", _arhoE_var_number = "
    //           << _arhoE_var_number
    //           << std::endl;

    // There are at least two possibilites:
    // A.) We are asked to map a non-flow variable (for example, solid heat conduction temperature)
    // B.) We are asked to map a flow variable, but a coupling is missing and therefore
    // _arhoE_var_number is invalid_uint

    // In case A we should return invalid_uint, then the subclasses
    // will detect this and just return 0 for the Jacobian.
    //
    // In case B, we can't really do anything, because it may be
    // legitimate for _arhoE_var_number to be invalid_uint if we are
    // running the 2-eqn model...
    return libMesh::invalid_uint;
  }
}
