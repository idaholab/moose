/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSEnergyViscousFlux.h"

template <>
InputParameters
validParams<NSEnergyViscousFlux>()
{
  InputParameters params = validParams<NSKernel>();
  params.addClassDescription("Viscous flux terms in energy equation.");
  return params;
}

NSEnergyViscousFlux::NSEnergyViscousFlux(const InputParameters & parameters)
  : NSKernel(parameters), _vst_derivs(*this)
{
}

Real
NSEnergyViscousFlux::computeQpResidual()
{
  // (tau * u) * grad(phi)
  RealVectorValue velocity(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  RealVectorValue vec = _viscous_stress_tensor[_qp] * velocity;

  return vec * _grad_test[_i][_qp];
}

Real
NSEnergyViscousFlux::computeQpJacobian()
{
  // No dependence of this term on U_4!
  return 0.0;
}

Real
NSEnergyViscousFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (isNSVariable(jvar))
  {

    // Convenience variables
    const RealTensorValue & tau = _viscous_stress_tensor[_qp];

    Real rho = _rho[_qp];
    Real phij = _phi[_j][_qp];
    RealVectorValue U(_rho_u[_qp], _rho_v[_qp], _rho_w[_qp]);

    // Map jvar into the variable m for our problem, regardless of
    // how Moose has numbered things.
    unsigned m = mapVarNumber(jvar);

    // Compute Jacobian terms based on the value of m
    switch (m)
    {
      case 0: // Density
      {
        // Return value
        Real value = 0.0;

        for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
        {
          Real intermediate_value = 0.0;

          for (unsigned ell = 0; ell < LIBMESH_DIM; ++ell)
            intermediate_value +=
                (U(ell) / rho * (-tau(k, ell) * phij / rho + _vst_derivs.dtau(k, ell, 0)));

          // Hit accumulated value with test function
          value += intermediate_value * _grad_test[_i][_qp](k);
        } // end for k

        return value;
      }

      case 1:
      case 2:
      case 3: // Momentums
      {
        // Return value
        Real value = 0.0;

        // "local" version of m, mapped to 0, 1, 2, for indexing
        // into Point objects.
        const unsigned int m_local = m - 1;

        for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
        {
          Real intermediate_value = tau(k, m_local) * phij / rho;

          for (unsigned int ell = 0; ell < LIBMESH_DIM; ++ell)
            intermediate_value += _vst_derivs.dtau(k, ell, m) * U(ell) /
                                  rho; // Note: pass 'm' to dtau, it will convert it internally

          // Hit accumulated value with test function
          value += intermediate_value * _grad_test[_i][_qp](k);
        } // end for k

        return value;
      }

      default:
        return 0.0;
    } // end switch (m)
  }
  else
    return 0.0;
}
