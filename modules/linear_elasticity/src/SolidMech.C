#include "SolidMech.h"

template<>
InputParameters validParams<SolidMech>()
{
  InputParameters params = validParams<Kernel>();
  params.set<bool>("use_displaced_mesh") = false;
  params.addParam("constant_properties", false, "Whether or not some optimizations can be made because the properties are going to be constant on each subdomain.  Note that they can still change from one subdomain to the next.");
  
  return params;
}

SolidMech::SolidMech(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
   _E_prop(getMaterialProperty<Real>("youngs_modulus")),
   _nu_prop(getMaterialProperty<Real>("poissons_ratio")),
   _constant_properties(getParam<bool>("constant_properties"))
{}

void
SolidMech::subdomainSetup()
{
  if(_constant_properties)
  {
    _qp = 0;
    recomputeConstants();
  }
}


void
SolidMech::recomputeConstants()
{
  _E = _E_prop[_qp];
  _nu = _nu_prop[_qp];

  _c1 = _E*(1.-_nu)/(1.+_nu)/(1.-2.*_nu);
  _c2 = _nu/(1.-_nu);
  _c3 = .5*(1.-2.*_nu)/(1.-_nu);

  _B11=TensorValue<Number>(1., 0., 0.,
                           0., _c3, 0.,
                           0., 0., _c3);
  
  _B12=TensorValue<Number>(0., _c2, 0.,
                           _c3, 0., 0.,
                           0., 0., 0.);

  _B13=TensorValue<Number>(0., 0., _c2,
                           0., 0., 0.,
                           _c3, 0., 0.);

  
  _B21=TensorValue<Number>(0., _c3, 0.,
                           _c2, 0., 0.,
                           0., 0., 0.);

  _B22=TensorValue<Number>(_c3, 0., 0.,
                           0., 1., 0.,
                           0., 0., _c3);
  
  _B23=TensorValue<Number>(0., 0., 0.,
                           0., 0., _c2,
                           0., _c3, 0.);


    
  _B31=TensorValue<Number>(0., 0., _c3,
                           0., 0., 0.,
                           _c2, 0., 0.);

  _B32=TensorValue<Number>(0., 0., 0.,
                           0., 0., _c3,
                           0., _c2, 0.);

  _B33=TensorValue<Number>(_c3, 0., 0.,
                           0., _c3, 0.,
                           0., 0., 1.);
}
