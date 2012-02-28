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
   _has_MJac(getParam<bool>("has_MJac")),
   _DM(_has_MJac ? &getMaterialProperty<Real>(_Dmob_name) : NULL),
   _implicit(getParam<bool>("implicit"))
{  
}

/*RealGradient  //Use This an example of the the function should look like
CHBulk::computeGradDFDCons(PFFunctionType type, Real c, RealGradient grad_c)
{
  switch (type)
  {
  case Residual:
    return 3*c*c*grad_c - grad_c; // return Residual value
    
  case Jacobian: 
    return 6*c*_phi[_j][_qp]*grad_c + 3*c*c*_grad_phi[_j][_qp] - _grad_phi[_j][_qp]; //return Jacobian value
    //return 0.0;
    
  }
  
  mooseError("Invalid type passed in");
}
*/

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
    mooseError("Grad u old no longer available.  Go talk to Derek about it.");
    
//    c = _u_old[_qp];
//    grad_c = _grad_u_old[_qp];
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
    mooseError("Grad u old no longer available.  Go talk to Derek about it.");

//    c = _u_old[_qp];
//    grad_c = _grad_u_old[_qp];
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
