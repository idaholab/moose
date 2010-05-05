#include "NavierStokesMaterial.h"

template<>
InputParameters validParams<NavierStokesMaterial>()
{
  InputParameters params = validParams<Material>();

  //Default is Air
  params.set<Real>("R")=287.04; // J/kgK
  params.set<Real>("gamma")=1.405;
  params.set<Real>("Pr")=0.71;
  
  return params;
}

NavierStokesMaterial::NavierStokesMaterial(std::string name,
                                           MooseSystem & moose_system,
                                           InputParameters parameters)
  :Material(name, moose_system, parameters),
    _has_u(isCoupled("u")),

    _u(coupledVal("u")),
    _grad_u(coupledGrad("u")),
    
    _has_v(isCoupled("v")),
    _v(coupledVal("v")),
    _grad_v(coupledGrad("v")),
    
    _has_w(isCoupled("w")),
    _w(_has_w ? coupledVal("w") : _zero),
    _grad_w(_has_w ? coupledGrad("w") : _grad_zero),
    
    _has_pe(isCoupled("pe")),
    _pe(coupledVal("pe")),
    _grad_pe(coupledGrad("pe")),

    _viscous_stress_tensor(declareTensorProperty("viscous_stress_tensor")),
    _thermal_conductivity(declareRealProperty("thermal_conductivity")),
    _pressure(declareRealProperty("pressure")),

    _gamma(declareConstantRealProperty("gamma")),
    _c_v(declareConstantRealProperty("c_v")),
    _c_p(declareConstantRealProperty("c_p")),
    _R(declareConstantRealProperty("R")),
    _Pr(declareConstantRealProperty("Pr")),

    //Declared here but _not_ calculated here
    _dynamic_viscocity(declareRealProperty("dynamic_viscocity")),
    
    _R_param(parameters.get<Real>("R")),
    _gamma_param(parameters.get<Real>("gamma")),
    _Pr_param(parameters.get<Real>("Pr"))
  {
    _gamma = _gamma_param;
    _R = _R_param;
    _Pr = _Pr_param;
    
    _c_v = _R / (_gamma_param - 1);

    _c_p = _gamma_param * _c_v;

    //Load these up in a vector for convenience
    _vel_grads.resize(3);

    _vel_grads[0] = &_grad_u;
    _vel_grads[1] = &_grad_v;
    _vel_grads[2] = &_grad_w;
  }


/**
 * Must be called _after_ the child class computes dynamic_viscocity.
 */
void
NavierStokesMaterial::computeProperties()
{  
  for(unsigned int qp=0; qp<_qrule->n_points(); qp++)
  {  
    /******* Viscous Stress Tensor *******/
    //Technically... this _is_ the transpose (since we are loading these by rows)
    //But it doesn't matter....
    RealTensorValue grad_outter_u(_grad_u[qp],_grad_v[qp],_grad_w[qp]);

    grad_outter_u += grad_outter_u.transpose();

    Real div_vel = 0;
    for(unsigned int i=0;i<3;i++)
      div_vel += (*_vel_grads[i])[qp](i);

    //Add diagonal terms
    for(unsigned int i=0;i<3;i++)
      grad_outter_u(i,i) -= (2.0/3.0) * div_vel;

    grad_outter_u *= _dynamic_viscocity[qp];
    
    _viscous_stress_tensor[qp] = grad_outter_u;

    
    /******* Pressure *******/
    _pressure[qp] = (_gamma_param - 1)*(_pe[qp] - (0.5 * (_u[qp]*_u[qp] + _v[qp]*_v[qp] + _w[qp]*_w[qp])));
      
    _thermal_conductivity[qp] = (_c_p * _dynamic_viscocity[qp]) / _Pr;
  }
}

