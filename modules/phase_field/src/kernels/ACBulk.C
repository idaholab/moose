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
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  params.addCoupledVar("args", "Vector of arguments to mobility");
  return params;
}

ACBulk::ACBulk(const std::string & name, InputParameters parameters) :
    DerivativeMaterialInterface<JvarMapInterface<KernelValue> >(name, parameters),
    _mob_name(getParam<MaterialPropertyName>("mob_name")),
    _L(getMaterialProperty<Real>(_mob_name)),
    _dLdop(getMaterialPropertyDerivative<Real>(_mob_name, _var.name()))
{
  // Get number of coupled variables
  unsigned int nvar = _coupled_moose_vars.size();

  // reserve space for derivatives
  _dLdarg.resize(nvar);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < nvar; ++i)
    _dLdarg[i] = &getMaterialPropertyDerivative<Real>(_mob_name, _coupled_moose_vars[i]->name());
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
  Real dFdop = computeDFDOP(Residual);

  return  _L[_qp] * dFdop;
}

Real
ACBulk::precomputeQpJacobian()
{
  Real dFdop = computeDFDOP(Residual);

  Real JdFdop = computeDFDOP(Jacobian);

  return _L[_qp] * JdFdop + _dLdop[_qp] * _phi[_j][_qp] * dFdop;
}

Real
ACBulk::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  unsigned int cvar;
  if (!mapJvarToCvar(jvar, cvar))
    return 0.0;

  return (*_dLdarg[cvar])[_qp] * _phi[_j][_qp] * computeDFDOP(Residual) * _test[_i][_qp];
}
