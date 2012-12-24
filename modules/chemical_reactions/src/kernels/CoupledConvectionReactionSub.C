#include "CoupledConvectionReactionSub.h"

#include "Material.h"

template<>
InputParameters validParams<CoupledConvectionReactionSub>()
{
  InputParameters params = validParams<Kernel>();
  
  params.addParam<Real>("weight",1.0,"Weight of the equilibrium species");
  params.addParam<Real>("log_k",0.0,"Equilibrium constant of dissociation equilibrium reaction");

  params.addParam<Real>("sto_u",1.0,"Stochiometric coef of the primary spceices the kernel operates on in the equilibrium reaction");
  params.addRequiredParam<std::vector<Real> >("sto_v","The stochiometric coefficients of coupled primary species in equilibrium reaction");
  params.addRequiredCoupledVar("p", "Pressure");
  params.addCoupledVar("v", "List of coupled primary species");
  return params;
}

CoupledConvectionReactionSub::CoupledConvectionReactionSub(const std::string & name, InputParameters parameters)

    // You must call the constructor of the base class first
  :Kernel(name,parameters),

    // coupledGradient will give us a reference to the gradient of another
    // variable in the computation.  We are going to use the gradient of p
    // to calculate our velocity vector.
   _weight(getParam<Real>("weight")),
   _log_k (getParam<Real>("log_k")),
   _sto_u(getParam<Real>("sto_u")),
   _sto_v(getParam<std::vector<Real> >("sto_v")),
   _cond(getMaterialProperty<Real>("conductivity")),
   _grad_p(coupledGradient("p"))
{
  int n = coupledComponents("v");
//  _vars.resize(n);
  _vals.resize(n);
  _grad_vals.resize(n);

  for (unsigned int i=0; i<_vals.size(); ++i)
  {
//    _vars[i] = coupled("v", i);
    _vals[i] = &coupledValue("v", i);
    _grad_vals[i] = &coupledGradient("v", i);
  }    

}

Real CoupledConvectionReactionSub::computeQpResidual()
{
  RealGradient _Darcy_vel=-_grad_p[_qp]*_cond[_qp];

  RealGradient _d_u = _sto_u*std::pow(_u[_qp],_sto_u-1.0)*_grad_u[_qp];

  RealGradient _d_var_sum(0.0,0.0,0.0);
  
  Real _d_v_u = std::pow(_u[_qp],_sto_u);
  
  if (_vals.size()) 
  {
    for (unsigned int i=0; i<_vals.size(); ++i)
    {
      _d_u *= std::pow((*_vals[i])[_qp],_sto_v[i]);

      RealGradient _d_var = _d_v_u * _sto_v[i]*std::pow((*_vals[i])[_qp],_sto_v[i]-1.0)*(*_grad_vals[i])[_qp];

      for (unsigned int j=0; j<_vals.size(); ++j)
      {
        if (j != i)
          _d_var *= std::pow((*_vals[j])[_qp],_sto_v[j]);
      }

      _d_var_sum += _d_var;
      
    }

  }
  
  return _weight*std::pow(10.0,_log_k)*_test[_i][_qp]*_Darcy_vel*(_d_u + _d_var_sum);
}

Real CoupledConvectionReactionSub::computeQpJacobian()
{
// the partial derivative of _grad_u is just _dphi[_j]

  RealGradient _Darcy_vel=-_grad_p[_qp]*_cond[_qp];
  
  RealGradient _d_u_1=_sto_u*std::pow(_u[_qp],_sto_u-1.0)*_grad_phi[_j][_qp];

  RealGradient _d_u_2=_phi[_j][_qp]*_sto_u*(_sto_u-1.0)*std::pow(_u[_qp],_sto_u-2.0)*_grad_u[_qp];

  RealGradient _d_u;
  
  RealGradient _d_var_sum(0.0,0.0,0.0);

  Real _d_v_u = _sto_u*std::pow(_u[_qp],_sto_u-1.0)*_phi[_j][_qp];
  
  if (_vals.size()) 
  {
    for (unsigned int i=0; i<_vals.size(); ++i)
    {
      _d_u_1 *= std::pow((*_vals[i])[_qp],_sto_v[i]);
      _d_u_2 *= std::pow((*_vals[i])[_qp],_sto_v[i]);
      
      RealGradient _d_var = _d_v_u * _sto_v[i]*std::pow((*_vals[i])[_qp],_sto_v[i]-1.0)*(*_grad_vals[i])[_qp];

      for (unsigned int j=0; j<_vals.size(); ++j)
      {
        if (j != i)
          _d_var *= std::pow((*_vals[j])[_qp],_sto_v[j]);
      }

      _d_var_sum += _d_var;
      
    }
  }

  _d_u = _d_u_1 + _d_u_2;

  return  _weight*std::pow(10.0,_log_k)*_test[_i][_qp]*_Darcy_vel*(_d_u + _d_var_sum);
}
