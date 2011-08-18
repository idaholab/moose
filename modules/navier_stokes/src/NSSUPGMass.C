#include "NSSUPGMass.h"

template<>
InputParameters validParams<NSSUPGMass>()
{
  // Initialize the params object from the base class
  InputParameters params = validParams<NSSUPGBase>();

  return params;
}



NSSUPGMass::NSSUPGMass(const std::string & name, InputParameters parameters)
    : NSSUPGBase(name, parameters)
{
}



Real NSSUPGMass::computeQpResidual()
{
  // From "Component SUPG contributions" section of the notes,
  // the mass equation is stabilized by taum and the gradient of
  // phi_i dotted with the momentum equation strong residuals.
  // Note that the momentum equation strong residuals are stored
  // in entries 1,2,3 of the "_strong_residuals" vector, regardless
  // of what dimension we're solving in.
  RealVectorValue Ru (_strong_residuals[_qp][1],
		      _strong_residuals[_qp][2],
		      _strong_residuals[_qp][3]);

  // Separate variable just for printing purposes...
  Real result = _taum[_qp] * (Ru * _grad_test[_i][_qp]);
  
  // Debugging: Print results only if they are nonzero
//  if (std::abs(result) > 0.)
//    std::cout << "SUPGMass[" << _qp << "]=" << result << std::endl;
  
  return result;
}


Real NSSUPGMass::computeQpJacobian()
{
  // This is the density equation, so pass the on-diagonal variable number
  return this->compute_jacobian(_rho_var_number);
}


Real NSSUPGMass::computeQpOffDiagJacobian(unsigned int jvar)
{
  return this->compute_jacobian(jvar);
}



Real NSSUPGMass::compute_jacobian(unsigned var)
{
  // Convert the Moose numbering to canonical NS variable numbering.
  unsigned  mapped_var_number = this->map_var_number(var);

  return _taum[_qp] * _grad_test[_i][_qp] * (_calA[_qp][mapped_var_number] * _grad_phi[_j][_qp]);
}
