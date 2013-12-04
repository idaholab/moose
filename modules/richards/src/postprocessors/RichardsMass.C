//  This post processor returns the mass value of a region.  It is used
//  to check that mass is conserved
//
#include "RichardsMass.h"

template<>
InputParameters validParams<RichardsMass>()
{
  InputParameters params = validParams<ElementIntegralVariablePostprocessor>();
  return params;
}

RichardsMass::RichardsMass(const std::string & name, InputParameters parameters) :
    ElementIntegralVariablePostprocessor(name, parameters),
    _porosity(getMaterialProperty<Real>("porosity")),
    _sat(getMaterialProperty<Real>("sat")),
    _density(getMaterialProperty<Real>("density"))
{
}

Real
RichardsMass::computeQpIntegral()
{
  return _porosity[_qp]*_density[_qp]*_sat[_qp];
}
