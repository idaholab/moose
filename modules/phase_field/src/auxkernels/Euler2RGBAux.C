#include "Euler2RGBAux.h"
#include "Euler2RGB.h"

template<>
InputParameters validParams<Euler2RGBAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addParam<unsigned int>("sd", 3, "An integer representing reference sample direction");
  return params;
}

Euler2RGBAux::Euler2RGBAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _sd(getParam<unsigned int>("sd")),
    _phi1(getMaterialProperty<Real>("phi1")),
    _phi(getMaterialProperty<Real>("phi")),
    _phi2(getMaterialProperty<Real>("phi2")),
    _phase(getMaterialProperty<unsigned int>("phase")),
    _sym(getMaterialProperty<unsigned int>("sym"))
{
}

Real
Euler2RGBAux::computeValue()
{
  // Call Euler2RGB Function and output RGB value as an integer
  // (only for quadrature point "0" within the element)
  return Euler2RGB(_sd, _phi1[0], _phi[0], _phi2[0], _phase[0], _sym[0]);
}
