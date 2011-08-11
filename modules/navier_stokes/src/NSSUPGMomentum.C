#include "NSSUPGMomentum.h"

template<>
InputParameters validParams<NSSUPGMomentum>()
{
  // Initialize the params object from the base class
  InputParameters params = validParams<NSSUPGBase>();

  params.addRequiredParam<unsigned>("component", "");

//  // Add extra required coupled variables
//  params.addRequiredCoupledVar("u", "");
//  params.addRequiredCoupledVar("v", "");
//  params.addCoupledVar("w", ""); // only required in 3D
  
  return params;
}



NSSUPGMomentum::NSSUPGMomentum(const std::string & name, InputParameters parameters)
    : NSSUPGBase(name, parameters),
      _component(getParam<unsigned>("component"))
{
}



Real NSSUPGMomentum::computeQpResidual()
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

    // _component'th entry of test function gradient
    Real dphi_dxk = _grad_test[_i][_qp](_component);

    // Vector object of momentum equation strong residuals
    RealVectorValue Ru (_strong_residuals[_qp][1],
			_strong_residuals[_qp][2],
			_strong_residuals[_qp][3]);

    // 1.) The mass-residual term:
    Real mass_coeff = 
      0.5*(_gamma-1.)*velmag2*dphi_dxk -
      vel(_component) * U_grad_phi;

    mass_term = _tauc[_qp] * mass_coeff * _strong_residuals[_qp][0];
    //std::cout << "mass_term[" << _qp << "]=" << mass_term << std::endl;

    // 2.) The momentum-residual term:
    Real mom_term1 = U_grad_phi * _strong_residuals[_qp][_component+1]; // <- momentum indices are 1,2,3
    Real mom_term2 = (1.-_gamma)*dphi_dxk*(vel*Ru);
    Real mom_term3 = vel(_component) * (_grad_test[_i][_qp]*Ru);

    mom_term = _taum[_qp] * (mom_term1 + mom_term2 + mom_term3);
    //std::cout << "mom_term[" << _qp << "]=" << mom_term << std::endl;

    // 3.) The energy-residual term:
    energy_term = _taue[_qp] * (_gamma-1.) * dphi_dxk * _strong_residuals[_qp][4];
    //std::cout << "energy_term[" << _qp << "]=" << energy_term << std::endl;
  }
  
  // For printing purposes only
  Real result = mass_term + mom_term + energy_term;
  // std::cout << "result[" << _qp << "]=" << result << std::endl;

  return result;
}


Real NSSUPGMomentum::computeQpJacobian()
{
  // TODO
  return 0.;
}


Real NSSUPGMomentum::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  // TODO
  return 0;
}
