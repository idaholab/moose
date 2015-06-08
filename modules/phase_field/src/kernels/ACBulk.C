/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ACBulk.h"

template<>
InputParameters validParams<ACBulk>()
{
  InputParameters params = validParams<KernelValue>();
  params.addClassDescription("Allen-Cahn Kernel");
  params.addParam<std::string>("mob_name", "L", "The mobility used with the kernel");
  return params;
}

ACBulk::ACBulk(const std::string & name, InputParameters parameters) :
    KernelValue(name, parameters),
    _mob_name(getParam<std::string>("mob_name")),
    _L(getMaterialProperty<Real>(_mob_name))
{
}

/*Real  //Use this as an example of how to create the function
ACBulk::computeDFDOP(PFFunctionType type)
{
  switch (type)
  {
  case Residual:
    return _u[_qp]*_u[_qp]*_u[_qp] - _u[_qp] ;

  case Jacobian:
    return _phi[_j][_qp]*(3*_u[_qp]*_u[_qp] - 1. );
  }

  mooseError("Invalid type passed in");
  }*/

Real
ACBulk::precomputeQpResidual()
{
  Real dFdeta = computeDFDOP(Residual);

  return  _L[_qp] * (dFdeta) ;
}

Real
ACBulk::precomputeQpJacobian()
{
  Real dFdeta = computeDFDOP(Jacobian);

  return _L[_qp] * (dFdeta);
}
