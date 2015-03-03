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
  InputParameters params = DerivativeKernelInterface<CHBulk>::validParams();
  params.addCoupledVar("args", "Vector of additional arguments to F");
  return params;
}

CHParsed::CHParsed(const std::string & name, InputParameters parameters) :
    DerivativeKernelInterface<CHBulk>(name, parameters)
{
  // reserve space for derivatives and gradients
  _second_derivatives.resize(_nvar+1);
  _third_derivatives.resize(_nvar+1);
  _grad_vars.resize(_nvar+1);

  // derivatives w.r.t. and gradients of the kernel variable
  _second_derivatives[0] = &getMaterialPropertyDerivative<Real>(_F_name, _var.name(), _var.name());
  _third_derivatives[0]  = &getMaterialPropertyDerivative<Real>(_F_name, _var.name(), _var.name(), _var.name());
  _grad_vars[0] = &(_grad_u);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < _nvar; ++i)
  {
    _second_derivatives[i+1] = &getMaterialPropertyDerivative<Real>(_F_name, _var.name(), _coupled_moose_vars[i]->name());
    _third_derivatives[i+1]  = &getMaterialPropertyDerivative<Real>(_F_name, _var.name(), _var.name(), _coupled_moose_vars[i]->name());
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
        res += (*_grad_vars[i])[_qp] * (*_second_derivatives[i])[_qp];
      return res;

    case Jacobian:
      res = _grad_phi[_j][_qp] * (*_second_derivatives[0])[_qp];
      for (unsigned int i = 0; i <= _nvar; ++i)
        res += _phi[_j][_qp] * (*_grad_vars[i])[_qp] * (*_third_derivatives[i])[_qp];
      return res;
  }

  mooseError("Internal error");
}
