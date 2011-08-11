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
  // TODO
  return 0.;
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
