#include "PFCTradMaterial.h"

template<>
InputParameters validParams<PFCTradMaterial>()
{
  InputParameters params = validParams<Material>();
  
  return params;
}

PFCTradMaterial::PFCTradMaterial(const std::string & name,
                 InputParameters parameters)
  :Material(name, parameters),
   _a(declareProperty<Real>("a")),
   _b(declareProperty<Real>("b")),
   _C0(declareProperty<Real>("C0")),
   _C2(declareProperty<Real>("C2")),
   _C4(declareProperty<Real>("C4"))
{
}

void
PFCTradMaterial::computeProperties()
{
  Real invSkm = 0.332;
  Real u_s = 0.72;

  _a[_qp] = 3.0/(2.0*u_s)*invSkm;
  _b[_qp] = 4.0/(30.0*u_s*u_s)*invSkm;
  _C0[_qp] = -10.9153;
  _C2[_qp] = 2.6; //Angstrom^2
  _C4[_qp] = -0.1459; //Angstrom^4
}

