#include "NSMomentumViscousFlux.h"
 

template<>
InputParameters validParams<NSMomentumViscousFlux>()
{
  // Initialize the params object from the base class
  InputParameters params = validParams<NSViscousFluxBase>();

  // component is a required parameter, so make it so!
  params.addRequiredParam<unsigned>("component", "");

  return params;
}




NSMomentumViscousFlux::NSMomentumViscousFlux(const std::string & name, InputParameters parameters)
    : NSViscousFluxBase(name, parameters),
      _component(getParam<unsigned>("component"))
{
}





Real NSMomentumViscousFlux::computeQpResidual()
{
  // Yay for less typing!
  RealTensorValue & vst = _viscous_stress_tensor[_qp];
  
  // _component'th column of vst...
  RealVectorValue vec(vst(0,_component),
		      vst(1,_component),
		      vst(2,_component));

  // ... dotted with grad(phi), note: sign is positive as this term was -div(tau) on the lhs
  return vec*_grad_test[_i][_qp];
}


Real NSMomentumViscousFlux::computeQpJacobian()
{
  Real value = 0.;

  // Set variable names as in the notes
  const unsigned k = _component;
  const unsigned m = _component+1; // _component = 0,1,2 -> m = 1,2,3 global variable number
  
  for (unsigned ell=0; ell<LIBMESH_DIM; ++ell)
    value += this->dtau(k, ell, m)*_grad_test[_i][_qp](ell);

  return value;
}


Real NSMomentumViscousFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  Real value = 0.;

  // Set variable names as in the notes
  const unsigned k = _component;

  // Map jvar into the variable m for our problem, regardless of
  // how Moose has numbered things. 
  unsigned m = 0;
  
  if (jvar == _rho_var_number)
    m = 0;
  else if (jvar == _rhou_var_number)
    m = 1;
  else if (jvar == _rhov_var_number)
    m = 2;
  else if (jvar == _rhow_var_number)
    m = 3;
  else if (jvar == _rhow_var_number)
    m = 4;

  for (unsigned ell=0; ell<LIBMESH_DIM; ++ell)
    value += this->dtau(k, ell, m)*_grad_test[_i][_qp](ell);

  return value;
}
