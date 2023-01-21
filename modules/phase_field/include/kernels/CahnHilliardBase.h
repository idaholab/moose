//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CHBulk.h"

/**
 * CahnHilliardBase implements the residual of the Cahn-Hilliard
 * equation in a general way that can be templated to a scalar or
 * tensor mobility.
 */
template <typename T>
class CahnHilliardBase : public CHBulk<T>
{
public:
  CahnHilliardBase(const InputParameters & parameters);

  static InputParameters validParams();
  virtual void initialSetup();

protected:
  typedef typename CHBulk<T>::PFFunctionType PFFunctionType;
  virtual RealGradient computeGradDFDCons(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  // Explicitly declare the use of the following members of the parent class
  // https://isocpp.org/wiki/faq/templates#nondependent-name-lookup-members
  using CHBulk<T>::_M;
  using CHBulk<T>::_i;
  using CHBulk<T>::_j;
  using CHBulk<T>::_qp;
  using CHBulk<T>::_var;
  using CHBulk<T>::_phi;
  using CHBulk<T>::_grad_u;
  using CHBulk<T>::_grad_phi;
  using CHBulk<T>::_grad_test;
  using CHBulk<T>::_coupled_moose_vars;
  using CHBulk<T>::_subproblem;
  using CHBulk<T>::_tid;

private:
  const unsigned int _nvar;
  std::vector<const MaterialProperty<Real> *> _second_derivatives;
  std::vector<const MaterialProperty<Real> *> _third_derivatives;
  std::vector<std::vector<const MaterialProperty<Real> *>> _third_cross_derivatives;
  std::vector<const VariableGradient *> _grad_vars;
};

template <typename T>
InputParameters
CahnHilliardBase<T>::validParams()
{
  InputParameters params = CHBulk<Real>::validParams();
  params.addClassDescription("Cahn-Hilliard Kernel that uses a DerivativeMaterial Free Energy");
  params.addRequiredParam<MaterialPropertyName>(
      "f_name", "Base name of the free energy function F defined in a DerivativeParsedMaterial");
  params.addCoupledVar("displacement_gradients",
                       "Vector of displacement gradient variables (see "
                       "Modules/PhaseField/DisplacementGradients "
                       "action)");
  return params;
}

template <typename T>
CahnHilliardBase<T>::CahnHilliardBase(const InputParameters & parameters)
  : CHBulk<T>(parameters),
    _nvar(_coupled_moose_vars.size()),
    _second_derivatives(_nvar + 1),
    _third_derivatives(_nvar + 1),
    _third_cross_derivatives(_nvar),
    _grad_vars(_nvar + 1)
{
  // derivatives w.r.t. and gradients of the kernel variable
  _second_derivatives[0] =
      &this->template getMaterialPropertyDerivative<Real>("f_name", _var.name(), _var.name());
  _third_derivatives[0] = &this->template getMaterialPropertyDerivative<Real>(
      "f_name", _var.name(), _var.name(), _var.name());
  _grad_vars[0] = &(_grad_u);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < _nvar; ++i)
  {
    const VariableName iname = _coupled_moose_vars[i]->name();
    if (iname == _var.name())
    {
      if (this->isCoupled("args"))
        this->paramError(
            "args", "The kernel variable should not be specified in the coupled `args` parameter.");
      else
        this->paramError("coupled_variables",
                         "The kernel variable should not be specified in the coupled "
                         "`coupled_variables` parameter.");
    }
    _second_derivatives[i + 1] =
        &this->template getMaterialPropertyDerivative<Real>("f_name", _var.name(), iname);
    _third_derivatives[i + 1] = &this->template getMaterialPropertyDerivative<Real>(
        "f_name", _var.name(), _var.name(), iname);

    _third_cross_derivatives[i].resize(_nvar);
    for (unsigned int j = 0; j < _nvar; ++j)
    {
      VariableName jname = _coupled_moose_vars[j]->name();
      _third_cross_derivatives[i][j] =
          &this->template getMaterialPropertyDerivative<Real>("f_name", _var.name(), iname, jname);
    }

    _grad_vars[i + 1] = &_subproblem.getStandardVariable(_tid, iname).gradSln();
  }
}

template <typename T>
void
CahnHilliardBase<T>::initialSetup()
{
  /**
   * Check if both the non-linear as well as the auxiliary variables variables
   * are coupled. Derivatives with respect to both types of variables contribute
   * the residual.
   */
  this->template validateCoupling<Real>("f_name", _var.name());
  this->template validateDerivativeMaterialPropertyBase<Real>("f_name");
}

template <typename T>
RealGradient
CahnHilliardBase<T>::computeGradDFDCons(PFFunctionType type)
{
  RealGradient res = 0.0;

  switch (type)
  {
    case CHBulk<T>::Residual:
      for (unsigned int i = 0; i <= _nvar; ++i)
        res += (*_grad_vars[i])[_qp] * (*_second_derivatives[i])[_qp];
      return res;

    case CHBulk<T>::Jacobian:
      res = _grad_phi[_j][_qp] * (*_second_derivatives[0])[_qp];
      for (unsigned int i = 0; i <= _nvar; ++i)
        res += _phi[_j][_qp] * (*_grad_vars[i])[_qp] * (*_third_derivatives[i])[_qp];
      return res;
  }

  mooseError("Internal error");
}

template <typename T>
Real
CahnHilliardBase<T>::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const unsigned int cvar = this->mapJvarToCvar(jvar);

  RealGradient J = _grad_u[_qp] * _phi[_j][_qp] * (*_third_derivatives[cvar + 1])[_qp] +
                   _grad_phi[_j][_qp] * (*_second_derivatives[cvar + 1])[_qp];

  for (unsigned int i = 0; i < _nvar; ++i)
    J += _phi[_j][_qp] * (*_grad_vars[i + 1])[_qp] * (*_third_cross_derivatives[i][cvar])[_qp];

  return CHBulk<T>::computeQpOffDiagJacobian(jvar) + _M[_qp] * _grad_test[_i][_qp] * J;
}
