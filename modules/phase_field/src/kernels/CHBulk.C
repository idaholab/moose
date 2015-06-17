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
  params.addCoupledVar("args", "Vector of arguments to mobility");
  return params;
}

CHBulk::CHBulk(const std::string & name, InputParameters parameters) :
    DerivativeMaterialInterface<JvarMapInterface<KernelGrad> >(name, parameters),
    _mob_name(getParam<std::string>("mob_name")),
    _M(getMaterialProperty<Real>(_mob_name)),
    _dMdc(getMaterialPropertyDerivative<Real>(_mob_name, _var.name()))
{
  //Get number of coupled variables
  unsigned int nvar = _coupled_moose_vars.size();

  // reserve space for derivatives
  _dMdarg.resize(nvar);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < nvar; ++i)
    _dMdarg[i] = &getMaterialPropertyDerivative<Real>(_mob_name, _coupled_moose_vars[i]->name());
}

RealGradient
CHBulk::precomputeQpResidual()
{
  return _M[_qp]*computeGradDFDCons(Residual);
}

RealGradient
CHBulk::precomputeQpJacobian()
{
  RealGradient grad_value = _M[_qp]*computeGradDFDCons(Jacobian)
                            + _dMdc[_qp]*_phi[_j][_qp]*computeGradDFDCons(Residual);

  return grad_value; //Return jacobian
}

Real
CHBulk::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  unsigned int cvar;
  if (!mapJvarToCvar(jvar, cvar))
    return 0.0;

  return (*_dMdarg[cvar])[_qp]*_phi[_j][_qp]*computeGradDFDCons(Residual)*_grad_test[_i][_qp];
}
