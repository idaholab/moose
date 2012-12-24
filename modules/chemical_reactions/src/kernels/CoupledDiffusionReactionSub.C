#include "CoupledDiffusionReactionSub.h"

#include "Material.h"

template<>
InputParameters validParams<CoupledDiffusionReactionSub>()
{
  InputParameters params = validParams<Kernel>();
  
  params.addParam<Real>("weight",1.0,"Weight of equilibrium species concentration in the primary species concentration");
  params.addParam<Real>("log_k",0.0,"Equilibrium constant of the equilbrium reaction in dissociation form");
  params.addParam<Real>("sto_u",1.0,"Stochiometric coef of the primary species this kernel operates on in the equilibrium reaction");
  params.addRequiredParam<std::vector<Real> >("sto_v","The stochiometric coefficients of coupled primary species");
  params.addCoupledVar("v", "List of coupled primary species in this equilibrium species");
  return params;
}

CoupledDiffusionReactionSub::CoupledDiffusionReactionSub(const std::string & name, InputParameters parameters)
  :Kernel(name,parameters),
   _diffusivity(getMaterialProperty<Real>("diffusivity")),
   _weight(getParam<Real>("weight")),
   _log_k(getParam<Real>("log_k")),
   _sto_u(getParam<Real>("sto_u")),
   _sto_v(getParam<std::vector<Real> >("sto_v"))
{
  int n = coupledComponents("v");
  _vals.resize(n);
  _grad_vals.resize(n);

  for (unsigned int i=0; i<_vals.size(); ++i)
  {
    _vals[i] = &coupledValue("v", i);
    _grad_vals[i] = &coupledGradient("v", i);
  }    

}

Real CoupledDiffusionReactionSub::computeQpResidual()
{
  RealGradient _diff1 = _sto_u*std::pow(_u[_qp],_sto_u-1.0)*_grad_u[_qp];
  if (_vals.size())
  {
    for (unsigned int i=0; i<_vals.size(); ++i)
    {
      _diff1 *= std::pow((*_vals[i])[_qp],_sto_v[i]);
    }
  }
  
  RealGradient _diff2_sum(0.0,0.0,0.0);

  Real _d_val = std::pow(_u[_qp],_sto_u);
  
  if (_vals.size()) 
  {
    
    for (unsigned int i=0; i<_vals.size(); ++i)
    {

      RealGradient _diff2 = _d_val*_sto_v[i]*std::pow((*_vals[i])[_qp],_sto_v[i]-1.0)*(*_grad_vals[i])[_qp];
      
      for (unsigned int j=0; j<_vals.size(); ++j)
      {
        if (j != i)
          _diff2 *= std::pow((*_vals[j])[_qp],_sto_v[j]);
      }
      _diff2_sum += _diff2;
      
    }
  }
  
    return  _weight*std::pow(10.0,_log_k)*_diffusivity[_qp]*_grad_test[_i][_qp]*(_diff1+_diff2_sum);
  
}

Real CoupledDiffusionReactionSub::computeQpJacobian()
{
  RealGradient _diff1_1 = _sto_u*std::pow(_u[_qp],_sto_u-1.0)*_grad_phi[_j][_qp];
  RealGradient _diff1_2 = _phi[_j][_qp]*_sto_u*(_sto_u-1.0)*std::pow(_u[_qp],_sto_u-2.0)*_grad_u[_qp];
  if (_vals.size())
  {
    for (unsigned int i=0; i<_vals.size(); ++i)
    {
      _diff1_1 *= std::pow((*_vals[i])[_qp],_sto_v[i]);
      _diff1_2 *= std::pow((*_vals[i])[_qp],_sto_v[i]);
    }
  }

  RealGradient _diff1 = _diff1_1+_diff1_2;
  
  Real _d_val = _sto_u*std::pow(_u[_qp],_sto_u-1.0)*_phi[_j][_qp];
  

  RealGradient _diff2_sum(0.0,0.0,0.0);
  
  if (_vals.size()) 
  {
    
    for (unsigned int i=0; i<_vals.size(); ++i)
    {

      RealGradient _diff2 = _d_val*_sto_v[i]*std::pow((*_vals[i])[_qp],_sto_v[i]-1.0)*(*_grad_vals[i])[_qp];
      
      for (unsigned int j=0; j<_vals.size(); ++j)
      {
        if (j != i)
          _diff2 *= std::pow((*_vals[j])[_qp],_sto_v[j]);
      }
      _diff2_sum += _diff2;
      
    }
  }

  return  _weight*std::pow(10.0,_log_k)*_diffusivity[_qp]*_grad_test[_i][_qp]*(_diff1+_diff2_sum);

}

/*
Real CoupledDiffusionReactionSub::computeQpOffDiagJacobian(unsigned int jvar)
  {
    if(jvar == _v_var1)
    {
        RealGradient _diff1 = _sto_v0*std::pow(_u[_qp],_sto_v0-1.0)*_grad_u[_qp]*_sto_v1*std::pow(_v[_qp],_sto_v1-1.0)*std::pow(_w[_qp],_sto_v2)*_phi[_j][_qp];
  
        RealGradient _diff2_1 = std::pow(_u[_qp],_sto_v0)*_sto_v1*(_sto_v1-1.0)*std::pow(_v[_qp],_sto_v1-2.0)*_grad_v[_qp]*std::pow(_w[_qp],_sto_v2)*_phi[_j][_qp];
        RealGradient _diff2_2 = std::pow(_u[_qp],_sto_v0)*_sto_v1*std::pow(_v[_qp],_sto_v1-1.0)*_dphi[_j][_qp]*std::pow(_w[_qp],_sto_v2);
        RealGradient _diff2   = _diff2_1+_diff2_2;

        RealGradient _diff3= std::pow(_u[_qp],_sto_v0)*_sto_v1*std::pow(_v[_qp],_sto_v1-1.0)*_sto_v2*std::pow(_w[_qp],_sto_v2-1.0)*_grad_w[_qp]*_phi[_j][_qp];  

        return  _weight*std::pow(10.0,_log_k)*(*_diffusivity)[_qp]*_dtest[_i][_qp]*(_diff1+_diff2+_diff3);
        
    }
    else if(jvar == _v_var2)
    {
        RealGradient _diff1 = _sto_v0*std::pow(_u[_qp],_sto_v0-1.0)*_grad_u[_qp]*std::pow(_v[_qp],_sto_v1)*_sto_v2*std::pow(_w[_qp],_sto_v2-1.0)*_phi[_j][_qp];
  
        RealGradient _diff2 = std::pow(_u[_qp],_sto_v0)*_sto_v1*std::pow(_v[_qp],_sto_v1-1.0)*_grad_v[_qp]*_sto_v2*std::pow(_w[_qp],_sto_v2-1.0)*_phi[_j][_qp];

        RealGradient _diff3_1 = std::pow(_u[_qp],_sto_v0)*std::pow(_v[_qp],_sto_v1)*_sto_v2*(_sto_v2-1.0)*std::pow(_w[_qp],_sto_v2-2.0)*_grad_w[_qp]*_phi[_j][_qp];
        RealGradient _diff3_2 = std::pow(_u[_qp],_sto_v0)*std::pow(_v[_qp],_sto_v1)*_sto_v2*std::pow(_w[_qp],_sto_v2-1.0)*_dphi[_j][_qp];
        RealGradient _diff3 = _diff3_1 + _diff3_2;
        
        return  _weight*std::pow(10.0,_log_k)*(*_diffusivity)[_qp]*_dtest[_i][_qp]*(_diff1+_diff2+_diff3);
    }
    else
      return 0.0;
  }
*/
