//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSSUPGEnergy.h"

// FluidProperties includes
#include "IdealGasFluidProperties.h"

registerMooseObject("NavierStokesApp", NSSUPGEnergy);

InputParameters
NSSUPGEnergy::validParams()
{
  InputParameters params = NSSUPGBase::validParams();
  params.addClassDescription(
      "Compute residual and Jacobian terms form the SUPG terms in the energy equation.");
  return params;
}

NSSUPGEnergy::NSSUPGEnergy(const InputParameters & parameters) : NSSUPGBase(parameters) {}

Real
NSSUPGEnergy::computeQpResidual()
{
  // See "Component SUPG contributions" section of notes for details.

  // Values to be summed up and returned
  Real mass_term = 0.0;
  Real mom_term = 0.0;
  Real energy_term = 0.0;

  // Ratio of specific heats
  const Real gam = _fp.gamma();

  // Velocity vector
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // Velocity vector magnitude squared
  Real velmag2 = vel.norm_sq();

  // Velocity vector, dotted with the test function gradient
  Real U_grad_phi = vel * _grad_test[_i][_qp];

  // Vector object of momentum equation strong residuals
  RealVectorValue Ru(
      _strong_residuals[_qp][1], _strong_residuals[_qp][2], _strong_residuals[_qp][3]);

  // 1.) The mass-residual term:
  Real mass_coeff = (0.5 * (gam - 1.0) * velmag2 - _specific_total_enthalpy[_qp]) * U_grad_phi;

  mass_term = _tauc[_qp] * mass_coeff * _strong_residuals[_qp][0];

  // 2.) The momentum-residual term:
  Real mom_term1 = _specific_total_enthalpy[_qp] * (_grad_test[_i][_qp] * Ru);
  Real mom_term2 = (1.0 - gam) * U_grad_phi * (vel * Ru);

  mom_term = _taum[_qp] * (mom_term1 + mom_term2);

  // 3.) The energy-residual term:
  energy_term = _taue[_qp] * gam * U_grad_phi * _strong_residuals[_qp][4];

  // For printing purposes only
  Real result = mass_term + mom_term + energy_term;
  // Moose::out << "result[" << _qp << "]=" << result << std::endl;

  return result;
}

Real
NSSUPGEnergy::computeQpJacobian()
{
  // This is the energy equation, so pass the on-diagonal variable number.
  return computeJacobianHelper(_rho_et_var_number);
}

Real
NSSUPGEnergy::computeQpOffDiagJacobian(unsigned int jvar)
{
  return computeJacobianHelper(jvar);
}

Real
NSSUPGEnergy::computeJacobianHelper(unsigned var)
{
  if (isNSVariable(var))
  {
    // Convert the Moose numbering to canonical NS variable numbering.
    unsigned mapped_var_number = mapVarNumber(var);

    // Convenience vars

    // Velocity vector
    RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

    // Velocity vector magnitude squared
    Real velmag2 = vel.norm_sq();

    // Ratio of specific heats
    const Real gam = _fp.gamma();

    // Shortcuts for shape function gradients at current qp.
    RealVectorValue grad_test_i = _grad_test[_i][_qp];
    RealVectorValue grad_phi_j = _grad_phi[_j][_qp];

    // ...

    // 1.) taum- and taue-proportional terms present for any variable:

    //
    // Art. Diffusion matrix for taum-proportional term = (diag(H) + (1-gam)*S) * A_{ell}
    //
    RealTensorValue mom_mat;
    mom_mat(0, 0) = mom_mat(1, 1) = mom_mat(2, 2) = _specific_total_enthalpy[_qp]; // (diag(H)
    mom_mat += (1. - gam) * _calC[_qp][0] * _calC[_qp][0].transpose();             //  + (1-gam)*S)
    mom_mat = mom_mat * _calA[_qp][mapped_var_number];                             // * A_{ell}
    Real mom_term = _taum[_qp] * grad_test_i * (mom_mat * grad_phi_j);

    //
    // Art. Diffusion matrix for taue-proportinal term = gam * E_{ell},
    // where E_{ell} = C_k * E_{k ell} for any k, summation over k *not* implied.
    //
    RealTensorValue ene_mat = gam * _calC[_qp][0] * _calE[_qp][0][mapped_var_number];
    Real ene_term = _taue[_qp] * grad_test_i * (ene_mat * grad_phi_j);

    // 2.) Terms only present if the variable is one of the momentums
    Real mass_term = 0.;

    switch (mapped_var_number)
    {
      case 1:
      case 2:
      case 3:
      {
        // Variable for zero-based indexing into local matrices and vectors.
        unsigned m_local = mapped_var_number - 1;

        //
        // Art. Diffusion matrix for tauc-proportional term = (0.5*(gam-1.)*velmag2 - H)*C_m
        //
        RealTensorValue mass_mat =
            (0.5 * (gam - 1.) * velmag2 - _specific_total_enthalpy[_qp]) * _calC[_qp][m_local];
        mass_term = _tauc[_qp] * grad_test_i * (mass_mat * grad_phi_j);

        // Don't even need to break, no other cases to fall through to...
        break;
      }
    }

    // Sum up values and return
    return mass_term + mom_term + ene_term;
  }
  else
    return 0.0;
}
