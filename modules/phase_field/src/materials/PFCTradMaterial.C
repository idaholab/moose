#include "PFCTradMaterial.h"

template<>
InputParameters validParams<PFCTradMaterial>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredParam<unsigned int>("order","This is the order of the polynomial used for correlation function");
  
  
  return params;
}

PFCTradMaterial::PFCTradMaterial(const std::string & name,
                 InputParameters parameters)
  :Material(name, parameters),
   _order(getParam<unsigned int>("order")),
   _M(declareProperty<Real>("M")),
   _a(declareProperty<Real>("a")),
   _b(declareProperty<Real>("b")),
   _C0(declareProperty<Real>("C0")),
   _C2(declareProperty<Real>("C2")),
   _C4(declareProperty<Real>("C4")),
   _C6(declareProperty<Real>("C6")),
   _C8(declareProperty<Real>("C8"))
{
}

void
PFCTradMaterial::computeQpProperties()
{
  Real invSkm = 0.332;
  Real u_s = 0.72;

  _M[_qp] = 1.0;
  _a[_qp] = 3.0/(2.0*u_s)*invSkm;
  _b[_qp] = 4.0/(30.0*u_s*u_s)*invSkm;


  if (_order == 4)
  {
    _C0[_qp] = -10.9153;
    _C2[_qp] = 2.6; //Angstrom^2
    _C4[_qp] = 0.1459; //Angstrom^4, would be negative but coefficient term is negative
  }
  else if (_order == 8)
  {
    _C0[_qp] = -49.0;
    _C2[_qp] = 19.54624209; //Angstrom^2
    _C4[_qp] = 2.998730464; //Angstrom^4, would be negative but coefficient term is negative
    _C6[_qp] = 0.2134499536; //Angstrom^6
    _C8[_qp] = 0.005988904704; //Angstrom^8, would be negative but coefficient term is negative
  }
  else
  {
    
  }
  
  
}

