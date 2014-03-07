//  This post processor returns the mass value of an element.  It is used
//  to check that mass is conserved (per the evolving density calculation)
//  when volume changes occur.
//
#include "Mass.h"

template<>
InputParameters validParams<Mass>()
{
  InputParameters params = validParams<ElementIntegralVariablePostprocessor>();
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

Mass::Mass(const std::string & name, InputParameters parameters) :
    ElementIntegralVariablePostprocessor(name, parameters),
    _density(getMaterialProperty<Real>("density"))

{
}

Real
Mass::computeQpIntegral()
{
  return _density[_qp];
}
