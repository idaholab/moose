#include "Euler2RGBAux.h"
#include "Euler2RGB.h"

template<>
InputParameters validParams<Euler2RGBAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<unsigned int>("sd", "An integer representing reference sample direction");
  return params;
}

Euler2RGBAux::Euler2RGBAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),

    _sd(getParam<unsigned int>("sd")),
    _phi1(getMaterialProperty<Real>("phi1")),
    _PHI(getMaterialProperty<Real>("PHI")),
    _phi2(getMaterialProperty<Real>("phi2")),
    _phase(getMaterialProperty<unsigned int>("phase")),
    _sym(getMaterialProperty<unsigned int>("sym"))
{
}

Real
Euler2RGBAux::computeValue()
{
  unsigned int sd = _sd;
  Real phi1 = _phi1[0];
  Real PHI = _PHI[0];
  Real phi2 = _phi2[0];
  unsigned int phase = _phase[0];
  unsigned int sym = _sym[0];

  // Call Euler2RGB Function and output RGB value as an integer (only for quadrature point "0" within the element)
  return Euler2RGB(sd, phi1, PHI, phi2, phase, sym);
}
