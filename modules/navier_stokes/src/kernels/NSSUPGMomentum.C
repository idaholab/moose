//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSSUPGMomentum.h"

// FluidProperties includes
#include "IdealGasFluidProperties.h"

registerMooseObject("NavierStokesApp", NSSUPGMomentum);

InputParameters
NSSUPGMomentum::validParams()
{
  InputParameters params = NSSUPGBase::validParams();
  params.addClassDescription(
      "Compute residual and Jacobian terms form the SUPG terms in the momentum equation.");
  params.addRequiredParam<unsigned int>("component", "");
  return params;
}

NSSUPGMomentum::NSSUPGMomentum(const InputParameters & parameters)
  : NSSUPGBase(parameters), _component(getParam<unsigned>("component"))
{
}

Real
NSSUPGMomentum::computeQpResidual()
{
  // See "Component SUPG contributions" section of notes for details.

  // Values to be summed up and returned
  Real mass_term = 0.0;
  Real mom_term = 0.0;
  Real energy_term = 0.0;

  // Velocity vector
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // Velocity vector magnitude squared
  Real velmag2 = vel.norm_sq();

  // Ratio of specific heats
  const Real gam = _fp.gamma();

  // Velocity vector, dotted with the test function gradient
  Real U_grad_phi = vel * _grad_test[_i][_qp];

  // _component'th entry of test function gradient
  Real dphi_dxk = _grad_test[_i][_qp](_component);

  // Vector object of momentum equation strong residuals
  RealVectorValue Ru(
      _strong_residuals[_qp][1], _strong_residuals[_qp][2], _strong_residuals[_qp][3]);

  // 1.) The mass-residual term:
  Real mass_coeff = 0.5 * (gam - 1.0) * velmag2 * dphi_dxk - vel(_component) * U_grad_phi;

  mass_term = _tauc[_qp] * mass_coeff * _strong_residuals[_qp][0];
  // Moose::out << "mass_term[" << _qp << "]=" << mass_term << std::endl;

  // 2.) The momentum-residual term:
  Real mom_term1 =
      U_grad_phi *
      _strong_residuals[_qp][_component + 1]; // <- momentum indices are 1,2,3, _component is 0,1,2
  Real mom_term2 = (1. - gam) * dphi_dxk * (vel * Ru);
  Real mom_term3 = vel(_component) * (_grad_test[_i][_qp] * Ru);

  mom_term = _taum[_qp] * (mom_term1 + mom_term2 + mom_term3);
  // Moose::out << "mom_term[" << _qp << "]=" << mom_term << std::endl;

  // 3.) The energy-residual term:
  energy_term = _taue[_qp] * (gam - 1.0) * dphi_dxk * _strong_residuals[_qp][4];

  // For printing purposes only
  Real result = mass_term + mom_term + energy_term;

  return result;
}

Real
NSSUPGMomentum::computeQpJacobian()
{
  // Set variable number for the computeJacobianHelper() function based on the
  // _component and the knowledge that this is the on-diagonal entry.
  unsigned int var_number[3] = {_rhou_var_number, _rhov_var_number, _rhow_var_number};

  // Call the common computeJacobianHelper() function
  return computeJacobianHelper(var_number[_component]);
}

Real
NSSUPGMomentum::computeQpOffDiagJacobian(unsigned int jvar)
{
  return computeJacobianHelper(jvar);
}

Real
NSSUPGMomentum::computeJacobianHelper(unsigned int var)
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
    // Art. Diffusion matrix for taum-proportional term = ( C_k + (1-gam)*C_k^T + diag(u_k) ) *
    // calA_{ell}
    //
    RealTensorValue mom_mat;
    mom_mat(0, 0) = mom_mat(1, 1) = mom_mat(2, 2) = vel(_component); // (diag(u_k)
    mom_mat += _calC[_qp][_component];                               //  + C_k
    mom_mat += (1.0 - gam) * _calC[_qp][_component].transpose();     //  + (1-gam)*C_k^T)
    mom_mat = mom_mat * _calA[_qp][mapped_var_number];               // * calA_{ell}
    Real mom_term =
        _taum[_qp] * grad_test_i * (mom_mat * grad_phi_j); // taum * grad(phi_i) * (M*grad(phi_j))

    //
    // Art. Diffusion matrix for taue-proportional term = (gam-1) * calE_km
    //
    RealTensorValue ene_mat = (gam - 1) * _calE[_qp][_component][mapped_var_number];
    Real ene_term =
        _taue[_qp] * grad_test_i * (ene_mat * grad_phi_j); // taue * grad(phi_i) * (M*grad(phi_j))

    // 2.) Terms only present if the variable is one of the momentums
    Real mass_term = 0.0;

    switch (mapped_var_number)
    {
      case 1:
      case 2:
      case 3:
      {
        // Variable for zero-based indexing into local matrices and vectors.
        unsigned m_local = mapped_var_number - 1;

        //
        // Art. Diffusion matrix for tauc-proportional term = 0.5*(gam - 1.0)*velmag2*D_km -
        // vel(_component)*C_m
        //
        RealTensorValue mass_mat;
        mass_mat(_component, m_local) = 0.5 * (gam - 1.0) * velmag2; // 0.5*(gam - 1.0)*velmag2*D_km
        mass_mat -= vel(_component) * _calC[_qp][m_local];           // vel(_component)*C_m
        mass_term = _tauc[_qp] * grad_test_i *
                    (mass_mat * grad_phi_j); // tauc * grad(phi_i) * (M*grad(phi_j))
      }
        // Nothing else to do if we are not a momentum...
    }

    // Sum up values and return
    return mass_term + mom_term + ene_term;
  }
  else
    return 0.0;
}
