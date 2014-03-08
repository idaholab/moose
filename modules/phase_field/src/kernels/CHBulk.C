#include "CHBulk.h"

template<>
InputParameters validParams<CHBulk>()
{
  InputParameters params = validParams<KernelGrad>();
  params.addParam<std::string>("mob_name","M","The mobility used with the kernel");
  params.addParam<std::string>("Dmob_name","DM","The D mobility used with the kernel");
  params.addParam<bool>("implicit",true,"The kernel will be run with implicit time integration");
  params.addParam<bool>("has_MJac",false,"Jacobian information for the mobility is defined");

  return params;
}

CHBulk::CHBulk(const std::string & name, InputParameters parameters)
  :KernelGrad(name, parameters),
   _mob_name(getParam<std::string>("mob_name")),
   _Dmob_name(getParam<std::string>("Dmob_name")),
   _M(getMaterialProperty<Real>(_mob_name)),
   _implicit(getParam<bool>("implicit")),
   _u_old(valueOld()),
   _grad_u_old(_implicit ? _grad_zero : gradientOld()),
   _has_MJac(getParam<bool>("has_MJac")),
   _DM(_has_MJac ? &getMaterialProperty<Real>(_Dmob_name) : NULL)
{
}

RealGradient
CHBulk::precomputeQpResidual()
{
  Real c;
  RealGradient grad_c;
  if (_implicit) // Changes values of c and grad_c depending on integration type
  {
    c = _u[_qp];
    grad_c = _grad_u[_qp];
  }
  else
  {
    c = _u_old[_qp];
    grad_c = _grad_u_old[_qp];
  }

  return _M[_qp] * computeGradDFDCons(Residual, c, grad_c);//Return residual
}

RealGradient
CHBulk::precomputeQpJacobian()
{
  Real c;
  RealGradient grad_c;
  if (_implicit) // Changes values of c and grad_c depending on integration type
  {
    c = _u[_qp];
    grad_c = _grad_u[_qp];
  }
  else
  {
    c = _u_old[_qp];
    grad_c = _grad_u_old[_qp];
  }

  RealGradient grad_value = 0.0;
  if (_implicit)
  {
    grad_value = _M[_qp] * computeGradDFDCons(Jacobian, c, grad_c);
    if (_has_MJac)
    {
      Real DMqp = (*_DM)[_qp];
      grad_value += DMqp*_phi[_j][_qp]*computeGradDFDCons(Residual, c, grad_c);
    }

  }

  return grad_value; //Return jacobian
}
