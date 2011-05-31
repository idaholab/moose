#include "SpecificHeatConstantVolumeAux.h"

template<>
InputParameters validParams<SpecificHeatConstantVolumeAux>()
{
  InputParameters params = validParams<AuxKernel>();
  
  // Specific heat is independent of other variables, we read its value from the input file
  params.addRequiredParam<Real>("specific_heat", "The specific heat at constant volume for air");

  return params;
}



SpecificHeatConstantVolumeAux::SpecificHeatConstantVolumeAux(const std::string & name, InputParameters parameters)
  :AuxKernel(name, parameters),
   _specific_heat(getParam<Real>("specific_heat"))
{
  // std::cout << "SpecificHeatConstantVolumeAux constructor: " << _specific_heat << std::endl;
}


Real
SpecificHeatConstantVolumeAux::computeValue()
{
  // std::cout << "SpecificHeatConstantVolumeAux::computeValue() " << _specific_heat << std::endl;
  
  // Return constant value, as read in from the input file.  The value is the same at every quadrature point.
  return _specific_heat;
}
