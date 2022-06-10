//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "NS.h"
#include "NSEnergyViscousBC.h"

registerMooseObject("NavierStokesApp", NSEnergyViscousBC);

InputParameters
NSEnergyViscousBC::validParams()
{
  InputParameters params = NSIntegratedBC::validParams();
  params.addRequiredCoupledVar(NS::temperature, "temperature");
  return params;
}

NSEnergyViscousBC::NSEnergyViscousBC(const InputParameters & parameters)
  : NSIntegratedBC(parameters),
    _grad_temperature(coupledGradient(NS::temperature)),
    _thermal_conductivity(getMaterialProperty<Real>("thermal_conductivity")),
    // Viscous stress tensor derivative computing object
    _vst_derivs(*this),
    // Temperature derivative computing object
    _temp_derivs(*this)
{
  // Store pointers to all variable gradients in a single vector.
  _gradU.resize(5);
  _gradU[0] = &_grad_rho;
  _gradU[1] = &_grad_rho_u;
  _gradU[2] = &_grad_rho_v;
  _gradU[3] = &_grad_rho_w;
  _gradU[4] = &_grad_rho_et;
}

Real
NSEnergyViscousBC::computeQpResidual()
{
  // n . (- k*grad(T) - tau*u) v

  // Velocity vector object
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // k*grad(T)
  RealVectorValue thermal_vec = _thermal_conductivity[_qp] * _grad_temperature[_qp];

  // tau*u
  RealVectorValue visc_vec = _viscous_stress_tensor[_qp] * vel;

  // Add everything up, dot with normal, hit with test function.
  return ((-thermal_vec - visc_vec) * _normals[_qp]) * _test[_i][_qp];
}

Real
NSEnergyViscousBC::computeQpJacobian()
{
  // See notes for this term, involves temperature Hessian
  Real thermal_term = 0.0;

  for (unsigned int ell = 0; ell < LIBMESH_DIM; ++ell)
  {
    Real intermediate_result = 0.;

    // The temperature Hessian contribution
    for (unsigned n = 0; n < 5; ++n)
      intermediate_result += _temp_derivs.get_hess(/*m=*/4, n) * (*_gradU[n])[_qp](ell);

    // Hit Hessian contribution with test function
    intermediate_result *= _phi[_j][_qp];

    // Add in the temperature gradient contribution
    intermediate_result += _temp_derivs.get_grad(/*rhoE=*/4) * _grad_phi[_j][_qp](ell);

    // Hit the result with the normal component, accumulate in thermal_term
    thermal_term += intermediate_result * _normals[_qp](ell);
  }

  // Hit thermal_term with thermal conductivity
  thermal_term *= _thermal_conductivity[_qp];

  return (-thermal_term) * _test[_i][_qp];
}

Real
NSEnergyViscousBC::computeQpOffDiagJacobian(unsigned jvar)
{
  if (isNSVariable(jvar))
  {
    // Note: This function requires both _vst_derivs *and* _temp_derivs

    // Convenience variables
    const RealTensorValue & tau = _viscous_stress_tensor[_qp];

    Real rho = _rho[_qp];
    Real phij = _phi[_j][_qp];
    RealVectorValue U(_rho_u[_qp], _rho_v[_qp], _rho_w[_qp]);

    // Map jvar into the variable m for our problem, regardless of
    // how Moose has numbered things.
    unsigned m = mapVarNumber(jvar);

    //
    // 1.) Thermal term derivatives
    //

    // See notes for this term, involves temperature Hessian
    Real thermal_term = 0.;

    for (unsigned ell = 0; ell < LIBMESH_DIM; ++ell)
    {
      Real intermediate_result = 0.;

      // The temperature Hessian contribution
      for (unsigned n = 0; n < 5; ++n)
        intermediate_result += _temp_derivs.get_hess(m, n) * (*_gradU[n])[_qp](ell);

      // Hit Hessian contribution with test function
      intermediate_result *= _phi[_j][_qp];

      // Add in the temperature gradient contribution
      intermediate_result += _temp_derivs.get_grad(m) * _grad_phi[_j][_qp](ell);

      // Hit the result with the normal component, accumulate in thermal_term
      thermal_term += intermediate_result * _normals[_qp](ell);
    }

    // Hit thermal_term with thermal conductivity
    thermal_term *= _thermal_conductivity[_qp];

    //
    // 2.) Viscous term derivatives
    //

    // Compute viscous term derivatives
    Real visc_term = 0.;

    switch (m)
    {
      case 0: // density
      {
        // Loop over k and ell as in the notes...
        for (const auto k : make_range(Moose::dim))
        {
          Real intermediate_value = 0.0;
          for (unsigned int ell = 0; ell < LIBMESH_DIM; ++ell)
            intermediate_value +=
                (U(ell) / rho * (-tau(k, ell) * phij / rho + _vst_derivs.dtau(k, ell, m)));

          // Hit accumulated value with normal component k.  We will multiply by test function at
          // the end of this routine...
          visc_term += intermediate_value * _normals[_qp](k);
        } // end for k

        break;
      } // end case 0

      case 1:
      case 2:
      case 3: // momentums
      {
        // Map m -> 0,1,2 as usual...
        unsigned int m_local = m - 1;

        // Loop over k and ell as in the notes...
        for (const auto k : make_range(Moose::dim))
        {
          Real intermediate_value = tau(k, m_local) * phij / rho;

          for (unsigned int ell = 0; ell < LIBMESH_DIM; ++ell)
            intermediate_value += _vst_derivs.dtau(k, ell, m) * U(ell) /
                                  rho; // Note: pass 'm' to dtau, it will convert it internally

          // Hit accumulated value with normal component k.
          visc_term += intermediate_value * _normals[_qp](k);
        } // end for k

        break;
      } // end case 1,2,3

      case 4: // energy
        mooseError("Shouldn't get here, this is the on-diagonal entry!");
        break;

      default:
        mooseError("Invalid m value.");
        break;
    }

    // Finally, sum up the different contributions (with appropriate
    // sign) multiply by the test function, and return.
    return (-thermal_term - visc_term) * _test[_i][_qp];
  }
  else
    return 0.0;
}
