/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SplitCHWRes.h"
template<>
InputParameters validParams<SplitCHWRes>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Split formulation Cahn-Hilliard Kernel for the chemical potential variable");
  params.addParam<std::string>("mob_name", "mobtemp", "The mobility used with the kernel");
  params.addCoupledVar("args", "Vector of arguments to mobility");
  return params;
}

SplitCHWRes::SplitCHWRes(const std::string & name, InputParameters parameters) :
    DerivativeMaterialInterface<JvarMapInterface<Kernel> >(name, parameters),
    _mob_name(getParam<std::string>("mob_name")),
    _mob(getMaterialProperty<Real>(_mob_name))
{
  //Get number of coupled variables
  unsigned int nvar = _coupled_moose_vars.size();

  // reserve space for derivatives
  _dmobdarg.resize(nvar);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < nvar; ++i)
    _dmobdarg[i] = &getMaterialPropertyDerivative<Real>(_mob_name, _coupled_moose_vars[i]->name());
}

Real
SplitCHWRes::computeQpResidual()
{
  return _mob[_qp]*_grad_u[_qp]*_grad_test[_i][_qp];
}

Real
SplitCHWRes::computeQpJacobian()
{
  return _mob[_qp]*_grad_phi[_j][_qp]*_grad_test[_i][_qp];
}

Real
SplitCHWRes::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  unsigned int cvar;
  if (!mapJvarToCvar(jvar, cvar))
    return 0.0;

  return (*_dmobdarg[cvar])[_qp]*_phi[_j][_qp]*_grad_u[_qp]*_grad_test[_i][_qp];
}
