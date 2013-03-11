#include "CHPFCRFF.h"
#include "MathUtils.h"
using namespace MathUtils;

template<>
InputParameters validParams<CHPFCRFF>()
{
  InputParameters params = validParams<CHBulk>();
  
  params.addRequiredCoupledVar("v", "Array of names of the real parts of the L variables");

  return params;
}

CHPFCRFF::CHPFCRFF(const std::string & name, InputParameters parameters)
  :CHBulk(name, parameters)
{
  _num_L = coupledComponents("v"); //Determine the number of L variables
  _grad_vals.resize(_num_L); //Resize variable array
  _vals_var.resize(_num_L);
  
  for (unsigned int i=0; i<_num_L; ++i)
  {
    _vals_var[i] = coupled("v",i);
    _grad_vals[i] = &coupledGradient("v", i);//Loop through grains and load coupled gradients into the arrays
  }
  
}

RealGradient
CHPFCRFF::computeGradDFDCons(PFFunctionType type, Real c, RealGradient grad_c)
{
  //We are actually calculating the term X u because we are pulling the u out of the mobility. The mobiltiy term shouldn't have u in it.
  RealGradient sum_grad_L;
  
  for (unsigned int i=0; i<_num_L; ++i)
    sum_grad_L += (*_grad_vals[i])[_qp];

  /* Real frac, dfrac;
  if (_u[_qp] < -1.0 + 1e-9)
  {
    frac = 1.0/1.0e-9;
    dfrac = -1.0/(1.0e-9*1.0e-9);
  }
  else
  {
    frac = 1.0/(1.0 + _u[_qp]);
    dfrac = -1.0/((1.0 + _u[_qp])*(1.0 + _u[_qp]));
    }*/
  
  switch (type)
  {
  case Residual:
    
    return _grad_u[_qp] - (1.0 + _u[_qp])*sum_grad_L;
    //return _grad_u[_qp]*frac - sum_grad_L;
    
  case Jacobian:
    return _grad_phi[_j][_qp] - _phi[_j][_qp]*sum_grad_L;
    //return _grad_phi[_j][_qp]*frac + _phi[_j][_qp]*_grad_u[_qp]*dfrac;
  }
  
  mooseError("Invalid type passed in");
}

Real
CHPFCRFF::computeQpOffDiagJacobian(unsigned int jvar)
{
  for (unsigned int i=0; i<_num_L; ++i)
    if (jvar == _vals_var[i])
    {
      RealGradient dsum_grad_L = _grad_phi[_j][_qp];
      RealGradient dGradDFDCons = - (1.0 + _u[_qp])*dsum_grad_L;
      //RealGradient dGradDFDCons = - dsum_grad_L;
      
      return _M[_qp]*dGradDFDCons*_grad_test[_i][_qp];
    }

  return 0.0;
}

      
