/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// Navier-Stokes includes
#include "NS.h"
#include "NSEnergyInviscidFlux.h"

// FluidProperties includes
#include "IdealGasFluidProperties.h"

template <>
InputParameters
validParams<NSEnergyInviscidFlux>()
{
  InputParameters params = validParams<NSKernel>();
  params.addClassDescription("This class computes the inviscid part of the energy flux.");
  params.addRequiredCoupledVar(NS::enthalpy, "total enthalpy");
  return params;
}

NSEnergyInviscidFlux::NSEnergyInviscidFlux(const InputParameters & parameters)
  : NSKernel(parameters), _enthalpy(coupledValue(NS::enthalpy))
{
}

Real
NSEnergyInviscidFlux::computeQpResidual()
{
  // H = total enthalpy = E + P/rho
  // => rho * u * H = rho * u ( E + P/rho)
  //                =       u ( rho*E + P)

  // velocity vector
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // Multiply vector U by the scalar value (rho*E + P) to get rho * U * H
  // vel *= (_u[_qp] + _pressure[_qp]);

  // Multiply velocity vector by the scalar (rho * H)
  vel *= (_rho[_qp] * _enthalpy[_qp]);

  // Return -1 * vel * grad(phi_i)
  return -(vel * _grad_test[_i][_qp]);
}

Real
NSEnergyInviscidFlux::computeQpJacobian()
{
  // Derivative of this kernel wrt rho*E
  const RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // Ratio of specific heats
  const Real gam = _fp.gamma();

  // -gamma * phi_j * (U*grad(phi_i))
  return -gam * _phi[_j][_qp] * (vel * _grad_test[_i][_qp]);
}

Real
NSEnergyInviscidFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (isNSVariable(jvar))
  {
    RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
    Real V2 = vel.norm_sq();

    // Ratio of specific heats
    const Real gam = _fp.gamma();

    // Derivative wrt density
    if (jvar == _rho_var_number)
      return -((0.5 * (gam - 1) * V2 - _enthalpy[_qp]) * _phi[_j][_qp] *
               (vel * _grad_test[_i][_qp]));

    // Derivatives wrt momentums
    else if ((jvar == _rhou_var_number) || (jvar == _rhov_var_number) || (jvar == _rhow_var_number))
    {
      // Map jvar into jlocal = {0,1,2}, regardless of how Moose has numbered things.
      unsigned jlocal = 0;

      if (jvar == _rhov_var_number)
        jlocal = 1;
      else if (jvar == _rhow_var_number)
        jlocal = 2;

      // Scale the velocity vector by the scalar (1-gamma)*vel(jlocal)
      vel *= (1.0 - gam) * vel(jlocal);

      // Add in the enthalpy in the jlocal'th entry
      vel(jlocal) += _enthalpy[_qp];

      // Return -1 * (vel * grad(phi_i)) * phi_j
      return -(vel * _grad_test[_i][_qp]) * _phi[_j][_qp];
    }

    else
    {
      std::ostringstream oss;
      oss << "Invalid jvar=" << jvar << " requested!\n"
          << "Did not match:\n"
          << " _rho_var_number =" << _rho_var_number << "\n"
          << " _rhou_var_number=" << _rhou_var_number << "\n"
          << " _rhov_var_number=" << _rhov_var_number << "\n"
          << " _rhow_var_number=" << _rhow_var_number << std::endl;
      mooseError(oss.str());
    }
  }
  else
    return 0.0;
}
