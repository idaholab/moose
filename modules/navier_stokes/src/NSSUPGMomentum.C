#include "NSSUPGMomentum.h"

template<>
InputParameters validParams<NSSUPGMomentum>()
{
  // Initialize the params object from the base class
  InputParameters params = validParams<NSSUPGBase>();

//  // Add extra required coupled variables
//  params.addRequiredCoupledVar("u", "");
//  params.addRequiredCoupledVar("v", "");
//  params.addCoupledVar("w", ""); // only required in 3D
  
  return params;
}



NSSUPGMomentum::NSSUPGMomentum(const std::string & name, InputParameters parameters)
    : NSSUPGBase(name, parameters)
{
}



Real NSSUPGMomentum::computeQpResidual()
{
  // TODO
  return 0.;
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
