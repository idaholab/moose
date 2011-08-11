#include "NSSUPGMass.h"

template<>
InputParameters validParams<NSSUPGMass>()
{
  // Initialize the params object from the base class
  InputParameters params = validParams<NSSUPGBase>();

//  // Add extra required coupled variables
//  params.addRequiredCoupledVar("u", "");
//  params.addRequiredCoupledVar("v", "");
//  params.addCoupledVar("w", ""); // only required in 3D
  
  return params;
}



NSSUPGMass::NSSUPGMass(const std::string & name, InputParameters parameters)
    : NSSUPGBase(name, parameters)
{
}



Real NSSUPGMass::computeQpResidual()
{
  // TODO
  
  // Compute tau values at current quadrature point.
  //this->compute_tau();

//  std::cout << "tauc=" << this->_tauc 
//	    << ", taum=" << this->_taum 
//	    << ", taue=" << this->_taue << std::endl;

  // Compute the strong residual values (sans viscous parts) at the current quadrature point.
  //this->compute_strong_residuals();

//  for (unsigned i=0; i<_strong_residuals.size(); ++i)
//    std::cout << _strong_residuals[i] << ", ";
//  std::cout << std::endl;

  return 0.;
}


Real NSSUPGMass::computeQpJacobian()
{
  // TODO
  return 0.;
}


Real NSSUPGMass::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  // TODO
  return 0;
}
