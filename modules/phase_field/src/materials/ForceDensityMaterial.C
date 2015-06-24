#include "ForceDensityMaterial.h"

template<>
InputParameters validParams<ForceDensityMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Calculating the force density acting on a grain");
  params.addCoupledVar("etas", "Array of coupled order parameters");
  params.addCoupledVar("c", "Concentration field");
  params.addParam<Real>("ceq",0.9816, "Equilibrium density");
  params.addParam<Real>("cgb",0.25, "Thresold Concentration for GB");
  params.addParam<Real>("k", 100.0, "stiffness constant");
  return params;
}

ForceDensityMaterial::ForceDensityMaterial(const std::string & name, InputParameters parameters) :
   Material(name,parameters),
   _c(coupledValue("c")),
   _ceq(getParam<Real>("ceq")),
   _cgb(getParam<Real>("cgb")),
   _k(getParam<Real>("k")),
   _ncrys(coupledComponents("etas")), //determine number of grains from the number of names passed in.  Note this is the actual number -1
   _vals(_ncrys), //Size variable arrays
   _grad_vals(_ncrys),
   _dF(declareProperty<std::vector<RealGradient> >("force_density"))
{
  //Loop through grains and load coupled variables into the arrays
  for (unsigned int i = 0; i < _ncrys; ++i)
  {
    _vals[i] = &coupledValue("etas", i);
    _grad_vals[i] = &coupledGradient("etas", i);
  }
}

void
ForceDensityMaterial::timestepSetup()
{
  for (_qp = 0; _qp < _dF.size(); ++_qp)
  _dF[_qp].resize(_ncrys);
}

void
ForceDensityMaterial::computeQpProperties()
{
  std::vector<Real> _product_etas(_ncrys);
  std::vector<RealGradient> _diff_grad_etas(_ncrys);
  Real product_eta_value = 0.0;

  for (unsigned int i = 0; i < _ncrys; ++i)
  {
    _product_etas[i] = 0.0;
    _diff_grad_etas[i] = 0.0;
    for (unsigned int j = 0; j < _ncrys; ++j)
    {
      if(i!=j)
      {
        _product_etas[i] += (*_vals[i])[_qp]*(*_vals[j])[_qp]; //Sum all other order parameters
        _diff_grad_etas[i] += ((*_grad_vals[i])[_qp]-(*_grad_vals[j])[_qp]);
      }
    }
    if(_product_etas[i] >= _cgb)
    {
      product_eta_value = 1.0;
    }
   _dF[_qp][i] = _k*(_c[_qp]-_ceq)*product_eta_value*_diff_grad_etas[i];
  }
}
