#include "NavierStokesMaterial.h"

template<>
InputParameters validParams<NavierStokesMaterial>()
{
  InputParameters params = validParams<Material>();

  //Default is Air
  
  // Raw libmesh style
  //params.set<Real>("R")=287.04; // J/kgK
  //params.set<Real>("gamma")=1.4;
  //params.set<Real>("Pr")=0.71;

  params.addRequiredParam<Real>("R", "Gas constant.");
  params.addRequiredParam<Real>("gamma", "Ratio of specific heats.");
  params.addRequiredParam<Real>("Pr", "Prandtl number.");

  params.addRequiredCoupledVar("u", "");
  params.addRequiredCoupledVar("v", "");
  params.addCoupledVar("w", ""); // not required in 2D
  params.addRequiredCoupledVar("c_v", ""); // c_v is now a aux variable, we may need it to compute e.g. c_p = g * c_v?

  return params;
}

NavierStokesMaterial::NavierStokesMaterial(const std::string & name,
                                           InputParameters parameters)
  :Material(name, parameters),
    _has_u(isCoupled("u")),
    _u(coupledValue("u")),
    _grad_u(coupledGradient("u")),
    
    _has_v(isCoupled("v")),
    _v(coupledValue("v")),
    _grad_v(coupledGradient("v")),
    
    _has_w(isCoupled("w")),
    _w(_has_w ? coupledValue("w") : _zero),
    _grad_w(_has_w ? coupledGradient("w") : _grad_zero),
    
//    _has_pe(isCoupled("pe")),
//    _pe(coupledValue("pe")),
//    _grad_pe(coupledGradient("pe")),

   /* density, needed by pressure calculation */
//    _has_p(isCoupled("p")),
//    _p(coupledValue("p")),
//    _grad_p(coupledGradient("p")),

   /* Specific heat, now a nodal aux variable */
    _has_c_v(isCoupled("c_v")),
    _c_v(coupledValue("c_v")),
   
    _viscous_stress_tensor(declareProperty<RealTensorValue>("viscous_stress_tensor")),
    _thermal_conductivity(declareProperty<Real>("thermal_conductivity")),
   //_pressure(declareProperty<Real>("pressure")),
   //_temperature(declareProperty<Real>("temperature")), // this is now a nodal aux variable
   //_temperature_gradient(declareProperty<RealGradient>("temperature_gradient")),

    _gamma(declareProperty<Real>("gamma")),
   // _c_v(declareProperty<Real>("c_v")), // _c_v made into a Nodal AuxVariable
    _c_p(declareProperty<Real>("c_p")),
    _R(declareProperty<Real>("R")),
    _Pr(declareProperty<Real>("Pr")),

    // Declared here but _not_ calculated here
    // (See e.g. derived class, bighorn/include/materials/FluidTC1.h)
    _dynamic_viscocity(declareProperty<Real>("dynamic_viscocity")),
    
    _R_param(getParam<Real>("R")),
    _gamma_param(getParam<Real>("gamma")),
    _Pr_param(getParam<Real>("Pr"))
  {
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
  for (unsigned int qp=0; qp<_qrule->n_points(); qp++)
  {  
    _gamma[qp] = _gamma_param;
    _R[qp] = _R_param;
    _Pr[qp] = _Pr_param;

    // _c_v[qp] = _R[qp] / (_gamma_param - 1); // Now an aux variable
    _c_p[qp] = _gamma_param * _c_v[qp];

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

    // Velocity vector, squared
    // Real V2 = _u[qp]*_u[qp] + _v[qp]*_v[qp] + _w[qp]*_w[qp];

    /******* Pressure *******/
    // _pressure[qp] = (_gamma_param - 1)*(_pe[qp] - 0.5 * _p[qp] * V2); // Now computed as an aux kernel
      
    /******* Temperature = (Internal Energy) / c_v *******/
    // If temperature is needed, it should be obtained from its AuxVariable
    // _temperature[qp] = ((_pe[qp]/_p[qp]) - 0.5*V2) / _c_v[qp]; 
    
    /******* Temperature Gradient *******/
    // This is non-trivial to compute directly, and I have a feeling that's the wrong 
    // way to do it anyway...
    // _temperature_gradient[qp] = RealGradient(dTdx, dTdy, dTdz);

    // Pr = (mu * cp) / k  ==>  k = (mu * cp) / Pr 
    _thermal_conductivity[qp] = (_c_p[qp] * _dynamic_viscocity[qp]) / _Pr[qp];

    // Tabulated values of thermal conductivity vs. Temperature (k increases slightly with T):
    // T (K)    k (W/m-K)
    // 273      0.0243
    // 373      0.0314
    // 473      0.0386
    // 573      0.0454
    // 673      0.0515
    
  }
}

