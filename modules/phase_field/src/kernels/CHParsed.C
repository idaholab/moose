/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CHParsed.h"

template<>
InputParameters validParams<CHParsed>()
{
  InputParameters params = validParams<CHBulk>();
  params.addClassDescription("Cahn-Hilliard Kernel that uses a DerivativeMaterial Free Energy");
  params.addRequiredParam<std::string>("f_name", "Base name of the free energy function F defined in a DerivativeParsedMaterial");
  return params;
}

CHParsed::CHParsed(const std::string & name, InputParameters parameters) :
    CHBulk(name, parameters),
    _F_name(getParam<std::string>("f_name")),
    _nvar(_coupled_moose_vars.size()),
    _second_derivatives(_nvar+1),
    _third_derivatives(_nvar+1),
    _third_cross_derivatives(_nvar),
    _grad_vars(_nvar+1)
{
  // derivatives w.r.t. and gradients of the kernel variable
  _second_derivatives[0] = &getMaterialPropertyDerivative<Real>(_F_name, _var.name(), _var.name());
  _third_derivatives[0]  = &getMaterialPropertyDerivative<Real>(_F_name, _var.name(), _var.name(), _var.name());
  _grad_vars[0] = &(_grad_u);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < _nvar; ++i)
  {
    std::string iname = _coupled_moose_vars[i]->name();
    _second_derivatives[i+1] = &getMaterialPropertyDerivative<Real>(_F_name, _var.name(), iname);
    _third_derivatives[i+1]  = &getMaterialPropertyDerivative<Real>(_F_name, _var.name(), _var.name(), iname);

    _third_cross_derivatives[i].resize(_nvar);
    for (unsigned int j = 0; j < _nvar; ++j)
    {
      std::string jname = _coupled_moose_vars[j]->name();
      _third_cross_derivatives[i][j] = &getMaterialPropertyDerivative<Real>(_F_name, _var.name(), iname, jname);
    }

    _grad_vars[i+1] = &(_coupled_moose_vars[i]->gradSln());
  }
}

RealGradient
CHParsed::computeGradDFDCons(PFFunctionType type)
{
  RealGradient res = 0.0;

  switch (type)
  {
    case Residual:
      for (unsigned int i = 0; i <= _nvar; ++i)
        res += (*_grad_vars[i])[_qp]*(*_second_derivatives[i])[_qp];
      return res;

    case Jacobian:
      res = _grad_phi[_j][_qp]*(*_second_derivatives[0])[_qp];
      for (unsigned int i = 0; i <= _nvar; ++i)
        res += _phi[_j][_qp]*(*_grad_vars[i])[_qp]*(*_third_derivatives[i])[_qp];
      return res;
  }

  mooseError("Internal error");
}

Real
CHParsed::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  unsigned int cvar;
  if (!mapJvarToCvar(jvar, cvar))
    return 0.0;

  RealGradient J =   _grad_u[_qp]*_phi[_j][_qp]*(*_third_derivatives[cvar+1])[_qp]
                   + _grad_phi[_j][_qp]*(*_second_derivatives[cvar+1])[_qp];

  for (unsigned int i = 0; i < _nvar; ++i)
    J += _phi[_j][_qp]*(*_grad_vars[i+1])[_qp]*(*_third_cross_derivatives[i][cvar])[_qp];

  return CHBulk::computeQpOffDiagJacobian(jvar) + _grad_test[_i][_qp]*_M[_qp]*J;
}
