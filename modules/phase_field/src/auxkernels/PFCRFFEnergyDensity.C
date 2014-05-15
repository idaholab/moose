
#include "PFCRFFEnergyDensity.h"

template<>
InputParameters validParams<PFCRFFEnergyDensity>()
{
   InputParameters params = validParams<AuxKernel>();
   params.addRequiredCoupledVar( "v", "Array of coupled variables" );
   params.addParam<Real>( "a", 1.0, "Modified Coefficent in Taylor Series Expanstion");
   params.addParam<Real>( "b", 1.0, "Modified Coefficent in Taylor Series Expanstion");
   params.addParam<Real>( "c", 1.0, "Modified Coefficent in Taylor Series Expanstion");
   params.addParam<unsigned int>( "num_exp_terms", 4, "This is the number of terms to use in the taylor series expansion");

   return params;
}

PFCRFFEnergyDensity::PFCRFFEnergyDensity(const std::string& name,
                                         InputParameters parameters) :
    AuxKernel( name, parameters),
    _order(coupledComponents("v")),
    _a(getParam<Real>("a")),
    _b(getParam<Real>("b")),
    _c(getParam<Real>("c")),
    _num_exp_terms(getParam<unsigned int>("num_exp_terms"))

{
  _vals.resize(_order);
  for (unsigned int i=0; i < _order; ++i)
    _vals[i] = &coupledValue("v", i);
}

Real
PFCRFFEnergyDensity::computeValue()
{
  Real val = 0;
  Real coef = 1.0;

  for(unsigned int i = 2; i < (2+_num_exp_terms); i++)
  {
    if (i==2)
      coef = _c;
    else if (i==3)
      coef = _a;
    else if (i==4)
      coef = _b;
    else
      coef = 1.0;

    val += coef*(pow(-1.0,i)/(i*(i - 1)))*pow((*_vals[0])[_qp],i);
  }

  // Loop Through Variables
  Real sumL = 0.0;
  for (unsigned int i = 1; i < _order; ++i)
    sumL += (*_vals[i])[_qp];

  val -= ((*_vals[0])[_qp]*sumL/2.0);

  return val;
}
