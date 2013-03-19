#include "CHPFCRFF.h"
#include "MathUtils.h"
using namespace MathUtils;

template<>
InputParameters validParams<CHPFCRFF>()
{
  InputParameters params = validParams<CHBulk>();
  
  params.addRequiredCoupledVar("v", "Array of names of the real parts of the L variables");
  MooseEnum log_options("tolerance, cancelation, expansion");
  params.addRequiredParam<MooseEnum>("log_approach", log_options, "Which approach will be used to handle the natural log");
  params.addParam<Real>("tol",1.0e-9,"Tolerance used when the tolerance approach is chosen");
  params.addParam<Real>("n_exp_terms",4,"Number of terms used in the Taylor expansion of the natural log term");

  return params;
}

CHPFCRFF::CHPFCRFF(const std::string & name, InputParameters parameters)
    :CHBulk(name, parameters),
     _log_approach(getParam<MooseEnum>("log_approach")),
     _tol(getParam<Real>("tol")),
     _n_exp_terms(getParam<Real>("n_exp_terms"))
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

  RealGradient sum_grad_L;
  
  for (unsigned int i=0; i<_num_L; ++i)
    sum_grad_L += (*_grad_vals[i])[_qp];
  
  Real frac, dfrac;
  Real ln_expansion = 0.0;

  switch(_log_approach)
  {
  case 0: //approach using tolerance
    if (1.0 + c < _tol)
    {
      frac = 1.0/_tol;
      dfrac = -1.0/(_tol*_tol);
    }
    else
    {
      frac = 1.0/(1.0 + c);
      dfrac = -1.0/((1.0 + c)*(1.0 + c));
    }
  case 2: //Approach using substitution
    for (unsigned int i=1; i<_n_exp_terms; ++i)
      ln_expansion += std::pow(double(-1),int(i+1))*std::pow(_u[_qp],int(i-1));
  }
  
  switch (type)
  {
  case Residual:

    switch(_log_approach)
    {
    case 0: //approach using tolerance
      return grad_c*frac - sum_grad_L;
    case 1: //approach using cancelation from the mobility
      return grad_c - (1.0 + c)*sum_grad_L;
    case 2: //appraoch using substitution
      return ln_expansion*grad_c - sum_grad_L;
    }

  case Jacobian:switch(_log_approach)
    {
    case 0: //approach using tolerance
      return _grad_phi[_j][_qp]*frac + _phi[_j][_qp]*grad_c*dfrac;
    case 1: //approach using cancelation from the mobility
      return _grad_phi[_j][_qp] - _phi[_j][_qp]*sum_grad_L;
    case 2: //appraoch using substitution
      Real Dln_expansion = 0.0;
      for (unsigned int i=2; i<_n_exp_terms; ++i)
        Dln_expansion += std::pow(double(-1),int(i+1))*(_n_exp_terms - 2.0)*std::pow(_u[_qp],int(i - 2));
      return ln_expansion*_grad_phi[_j][_qp] + _phi[_j][_qp]*Dln_expansion*grad_c;
    }
  }
  
  mooseError("Invalid type passed in");
}

Real
CHPFCRFF::computeQpOffDiagJacobian(unsigned int jvar)
{
  Real c = _u[_qp];
  
  for (unsigned int i=0; i<_num_L; ++i)
    if (jvar == _vals_var[i])
    {
      RealGradient dsum_grad_L = _grad_phi[_j][_qp];
      RealGradient dGradDFDCons;
      switch(_log_approach)
      {
      case 0: //approach using tolerance
        dGradDFDCons = -dsum_grad_L;
      case 1:  //approach using cancelation from the mobility
        dGradDFDCons = -(1.0 + c)*dsum_grad_L;
      case 2: //appraoch using substitution
        dGradDFDCons = -dsum_grad_L;
      }

      return _M[_qp]*dGradDFDCons*_grad_test[_i][_qp];
    }

  return 0.0;
}

      
