#include "ACGrGrPoly.h"

//#include "Material.h"

template<>
InputParameters validParams<ACGrGrPoly>()
{
  InputParameters params = validParams<ACBulk>();
  
  params.addRequiredCoupledVar("v", "Array of coupled variable names");
  params.addCoupledVar("bnds","Term distinguishing grain from grain boundary");
  
  return params;
}

ACGrGrPoly::ACGrGrPoly(const std::string & name, InputParameters parameters)
  :ACBulk(name,parameters),
   _mu(getMaterialProperty<Real>("mu")),
   _gamma(getMaterialProperty<Real>("gamma_asymm")),
   _has_bnds(isCoupled("bnds")),
   _bnds(_has_bnds ? coupledValue("bnds") : _zero)
{
  //Array of coupled variables is created in the constructor
  _ncrys = coupledComponents("v"); //determine number of grains from the number of names passed in.  Note this is the actual number -1
  _vals.resize(_ncrys); //Size variable arrays
  _vals_var.resize(_ncrys);
//  _gamma = 1.5;

  for (unsigned int i=0; i<_ncrys; ++i)
  {
    //Loop through grains and load coupled variables into the arrays
    _vals[i] = &coupledValue("v", i);
    _vals_var[i] = coupled("v",i);
  }  
  
}

Real
ACGrGrPoly::computeDFDOP(PFFunctionType type)
{
  Real SumEtaj = 0.0;
  if (_has_bnds && _bnds[_qp] >= 1.0 && _t_step > 1) //This is a possibly misguided attempt to speed up the calculation of SumEtaj
  {
    if (_u[_qp] < 0.0001 )
      SumEtaj = 1.0;
    //std::cout << "SumEtaj = " << SumEtaj << " u = " << _u[_qp] << " bnds = " << _bnds[_qp] << "||";
  }
  else
  {
    for (unsigned int i=0; i<_ncrys; ++i)
      SumEtaj += (*_vals[i])[_qp]*(*_vals[i])[_qp]; //Sum all other order parameters
  }

  //Calcualte either the residual or jacobian of the grain growth free energy
  switch (type)
  {
  case Residual:
    return _mu[_qp]*(_u[_qp]*_u[_qp]*_u[_qp] - _u[_qp] + 2.0*_gamma[_qp]*_u[_qp]*SumEtaj);

  case Jacobian:
    return _mu[_qp]*(_phi[_j][_qp]*(3*_u[_qp]*_u[_qp] - 1.0 + 2.0*_gamma[_qp]*SumEtaj));
  }

  mooseError("Invalid type passed in");
}

Real
ACGrGrPoly::computeQpOffDiagJacobian(unsigned int jvar)
{
  for (unsigned int i=0; i<_ncrys; ++i)
    if(jvar == _vals_var[i])
    {
      Real dSumEtaj = 2.0*(*_vals[i])[_qp]*_phi[_j][_qp]; //Derivative of SumEtaj

      Real dDFDOP = _mu[_qp]*2.0*_gamma[_qp]*_u[_qp]*dSumEtaj;
      
      return _L[_qp]*_test[_i][_qp]*dDFDOP;
    }

  return 0.0;
}
