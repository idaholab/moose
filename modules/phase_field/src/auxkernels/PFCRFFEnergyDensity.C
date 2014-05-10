
#include "PFCRFFEnergyDensity.h"

template<>
InputParameters validParams<PFCRFFEnergyDensity>()
{
   InputParameters params = validParams<AuxKernel>();
   params.addRequiredCoupledVar( "v", "Array of coupled variables" );
   params.addParam<Real>( "a", 1.0, "Modified Coefficent in Taylor Series Expanstion");
   params.addParam<Real>( "b", 1.0, "Modified Coefficent in Taylor Series Expanstion");
   params.addParam<Real>( "c", 1.0, "Modified Coefficent in Taylor Series Expanstion");

   return params;
}

PFCRFFEnergyDensity::PFCRFFEnergyDensity(const std::string& name,
                                         InputParameters parameters) :
    AuxKernel( name, parameters),
    _order(coupledComponents("v")),
    _a(getParam<Real>("a")),
    _b(getParam<Real>("b")),
    _c(getParam<Real>("c"))
{
  _vals.resize(_order);
  for (unsigned int i=0; i < _order; ++i)
    _vals[i] = &coupledValue("v", i);
}

Real
PFCRFFEnergyDensity::computeValue()
{
  Real val =   _c/2.0 * (*_vals[0])[_qp]
             + (_a/6.0 * std::pow((*_vals[0])[_qp], 2.0))
             + (_b/12.0 * std::pow((*_vals[0])[_qp], 3.0));

  // Loop Through Variables
  for (unsigned int i = 1; i < _order; ++i)
    val += (*_vals[i])[_qp];

  return val;
}
