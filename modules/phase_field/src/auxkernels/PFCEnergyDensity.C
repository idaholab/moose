
#include "PFCEnergyDensity.h"

template<>
InputParameters validParams<PFCEnergyDensity>()
{
   InputParameters params = validParams<AuxKernel>();
   params.addRequiredCoupledVar( "v", "Array of coupled variables" );

   return params;
}

PFCEnergyDensity::PFCEnergyDensity( const std::string& name, InputParameters parameters ) 
    : AuxKernel( name, parameters),
     _a(getMaterialProperty<Real>("a")),
      _b(getMaterialProperty<Real>("b")) 
{
  _order = coupledComponents("v");
  
  _vals.resize(_order);
  _coeff.resize(_order);

  std::string coeff_name_base = "C";

  for (unsigned int i=0; i<_order; ++i)
  {
    _vals[i] = &coupledValue("v", i);
    std::string coeff_name = coeff_name_base;
    std::stringstream out;
    out << i*2;
    coeff_name.append(out.str());
    std::cout << coeff_name << std::endl;
    _coeff[i] = &getMaterialProperty<Real>(coeff_name);
  }
}

Real
PFCEnergyDensity::computeValue()
{
  Real val = pow((*_vals[0])[_qp],2)*(1 - (*_coeff[0])[_qp])/2.0;

  // Loop Through Variables
  for (unsigned int i = 1; i < _order; i++)
    val += pow(-1.0,i+1)* (*_coeff[i])[_qp]* (*_vals[0])[_qp]* (*_vals[i])[_qp]/2.0;

  val += (_b[_qp]/12.0*pow((*_vals[0])[_qp],4)) - (_a[_qp]/6.0*pow((*_vals[0])[_qp],3));
  return val;
}
