#include "NSSUPGEnergy.h"

template<>
InputParameters validParams<NSSUPGEnergy>()
{
  // Initialize the params object from the base class
  InputParameters params = validParams<NSSUPGBase>();

//  // Add extra required coupled variables
//  params.addRequiredCoupledVar("u", "");
//  params.addRequiredCoupledVar("v", "");
//  params.addCoupledVar("w", ""); // only required in 3D
  
  return params;
}



NSSUPGEnergy::NSSUPGEnergy(const std::string & name, InputParameters parameters)
    : NSSUPGBase(name, parameters)
{
}



Real NSSUPGEnergy::computeQpResidual()
{
  // See "Component SUPG contributions" section of notes for details.
  
  // Values to be summed up and returned
  Real 
    mass_term = 0.,
    mom_term = 0.,
    energy_term = 0.;

  {
    // Velocity vector
    RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
    
    // Velocity vector magnitude squared
    Real velmag2 = vel.size_sq();

    // Velocity vector, dotted with the test function gradient
    Real U_grad_phi = vel*_grad_test[_i][_qp];

    // Vector object of momentum equation strong residuals
    RealVectorValue Ru (_strong_residuals[_qp][1],
			_strong_residuals[_qp][2],
			_strong_residuals[_qp][3]);

    // 1.) The mass-residual term:
    Real mass_coeff = 
      (0.5*(_gamma-1.)*velmag2 - _enthalpy[_qp]) * U_grad_phi;

    mass_term = _tauc[_qp] * mass_coeff * _strong_residuals[_qp][0];

    // 2.) The momentum-residual term:
    Real mom_term1 = _enthalpy[_qp]*(_grad_test[_i][_qp]*Ru);
    Real mom_term2 = (1.-_gamma)*U_grad_phi*(vel*Ru);

    mom_term = _taum[_qp] * (mom_term1 + mom_term2);

    // 3.) The energy-residual term:
    energy_term = _taue[_qp] * _gamma * U_grad_phi * _strong_residuals[_qp][4];
  }

  // For printing purposes only
  Real result = mass_term + mom_term + energy_term;
  // std::cout << "result[" << _qp << "]=" << result << std::endl;

  return result;
}


Real NSSUPGEnergy::computeQpJacobian()
{
  // TODO
  return 0.;
}


Real NSSUPGEnergy::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  // TODO
  return 0;
}
