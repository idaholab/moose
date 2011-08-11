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
  // These are now material properties
//  std::cout << "hsupg=" << this->_hsupg[_qp]
//	    << ", tauc=" << this->_tauc[_qp] 
//	    << ", taum=" << this->_taum[_qp] 
//	    << ", taue=" << this->_taue[_qp] << std::endl;

//  for (unsigned i=0; i<_strong_residuals[_qp].size(); ++i)
//    std::cout << _strong_residuals[_qp][i] << ", ";
//  std::cout << std::endl;

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
  //std::cout << "SUPGMass[" << _qp << "]=" << result << std::endl;
  
  return result;
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
