#include "PFCEnergyDensity.h"

template<>
InputParameters validParams<PFCEnergyDensity>()
{
   InputParameters params = validParams<AuxKernel>();
   params.addRequiredCoupledVar( "v", "Array of coupled variables" );
   return params;
}

PFCEnergyDensity::PFCEnergyDensity(const std::string& name,
                                   InputParameters parameters) :
    AuxKernel( name, parameters),
    _order(coupledComponents("v")),
    _a(getMaterialProperty<Real>("a")),
    _b(getMaterialProperty<Real>("b"))
{
  _vals.resize(_order);
  _coeff.resize(_order);

  std::string coeff_name_base = "C";

  for (unsigned int i = 0; i < _order; ++i)
  {
    _vals[i] = &coupledValue("v", i);
    std::string coeff_name = coeff_name_base;
    std::stringstream out;
    out << i*2;
    coeff_name.append(out.str());
    _console << coeff_name << std::endl;
    _coeff[i] = &getMaterialProperty<Real>(coeff_name);
  }
}

Real
PFCEnergyDensity::computeValue()
{
  Real val = std::pow((*_vals[0])[_qp],2)*(1 - (*_coeff[0])[_qp])/2.0;

  // Loop Through Variables
  // the sign of negative terms have been taken care of by changing the sign of the coefficients;
  for (unsigned int i = 1; i < _order; ++i)
    val += (*_coeff[i])[_qp] * (*_vals[0])[_qp] * (*_vals[i])[_qp]/2.0;

  val +=   (_b[_qp]/12.0 * std::pow((*_vals[0])[_qp], 4.0))
         - (_a[_qp]/6.0 * std::pow((*_vals[0])[_qp], 3.0));

  return val;
}
