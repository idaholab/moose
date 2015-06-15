/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CHBulk.h"

template<>
InputParameters validParams<CHBulk>()
{
  InputParameters params = validParams<KernelGrad>();
  params.addClassDescription("Cahn-Hilliard Kernel");
  params.addParam<std::string>("mob_name", "M", "The mobility used with the kernel");
  params.addParam<std::string>("Dmob_name", "DM", "The D mobility used with the kernel");
  params.addParam<bool>("has_MJac", false, "Jacobian information for the mobility is defined");
  return params;
}

CHBulk::CHBulk(const std::string & name, InputParameters parameters) :
    KernelGrad(name, parameters),
    _mob_name(getParam<std::string>("mob_name")),
    _Dmob_name(getParam<std::string>("Dmob_name")),
    _M(getMaterialProperty<Real>(_mob_name)),
    _has_MJac(getParam<bool>("has_MJac")),
    _DM(_has_MJac ? &getMaterialProperty<Real>(_Dmob_name) : NULL)
{
}

RealGradient
CHBulk::precomputeQpResidual()
{
  return _M[_qp] * computeGradDFDCons(Residual);
}

RealGradient
CHBulk::precomputeQpJacobian()
{
  RealGradient grad_value = 0.0;
  if (isImplicit())
  {
    grad_value = _M[_qp] * computeGradDFDCons(Jacobian);
    if (_has_MJac)
    {
      Real DMqp = (*_DM)[_qp];
      grad_value += DMqp*_phi[_j][_qp] * computeGradDFDCons(Residual);
    }
  }

  return grad_value; //Return jacobian
}
