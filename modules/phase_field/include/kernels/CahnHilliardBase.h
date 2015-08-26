/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CAHNHILLIARDBASE_H
#define CAHNHILLIARDBASE_H

#include "CHBulk.h"

/**
 * CahnHilliardBase implements the residual of the Cahn-Hilliard
 * equation in a general way that can be templated to a scalar or
 * tensor mobility.
 */
template<typename T>
class CahnHilliardBase : public CHBulk<T>
{
public:
  CahnHilliardBase(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  typedef typename CHBulk<T>::PFFunctionType PFFunctionType;
  virtual RealGradient computeGradDFDCons(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  const unsigned int _nvar;
  std::vector<const MaterialProperty<Real>* > _second_derivatives;
  std::vector<const MaterialProperty<Real>* > _third_derivatives;
  std::vector<std::vector<const MaterialProperty<Real>* > > _third_cross_derivatives;
  std::vector<VariableGradient *> _grad_vars;
};

template<typename T>
InputParameters
CahnHilliardBase<T>::validParams()
{
  InputParameters params = CHBulk<Real>::validParams();
  params.addClassDescription("Cahn-Hilliard Kernel that uses a DerivativeMaterial Free Energy");
  params.addRequiredParam<MaterialPropertyName>("f_name", "Base name of the free energy function F defined in a DerivativeParsedMaterial");
  return params;
}

template<typename T>
CahnHilliardBase<T>::CahnHilliardBase(const InputParameters & parameters) :
    CHBulk<T>(parameters),
    _nvar(this->_coupled_moose_vars.size()),
    _second_derivatives(_nvar+1),
    _third_derivatives(_nvar+1),
    _third_cross_derivatives(_nvar),
    _grad_vars(_nvar+1)
{
  // derivatives w.r.t. and gradients of the kernel variable
  _second_derivatives[0] = &this->template getMaterialPropertyDerivative<Real>("f_name", this->_var.name(), this->_var.name());
  _third_derivatives[0]  = &this->template getMaterialPropertyDerivative<Real>("f_name", this->_var.name(), this->_var.name(), this->_var.name());
  _grad_vars[0] = &(this->_grad_u);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < _nvar; ++i)
  {
    VariableName iname = this->_coupled_moose_vars[i]->name();
    _second_derivatives[i+1] = &this->template getMaterialPropertyDerivative<Real>("f_name", this->_var.name(), iname);
    _third_derivatives[i+1]  = &this->template getMaterialPropertyDerivative<Real>("f_name", this->_var.name(), this->_var.name(), iname);

    _third_cross_derivatives[i].resize(_nvar);
    for (unsigned int j = 0; j < _nvar; ++j)
    {
      VariableName jname = this->_coupled_moose_vars[j]->name();
      _third_cross_derivatives[i][j] = &this->template getMaterialPropertyDerivative<Real>("f_name", this->_var.name(), iname, jname);
    }

    _grad_vars[i+1] = &(this->_coupled_moose_vars[i]->gradSln());
  }
}

template<typename T>
RealGradient
CahnHilliardBase<T>::computeGradDFDCons(PFFunctionType type)
{
  RealGradient res = 0.0;
  unsigned int & qp = this->_qp;

  switch (type)
  {
    case CHBulk<T>::Residual:
      for (unsigned int i = 0; i <= _nvar; ++i)
        res += (*_grad_vars[i])[qp] * (*_second_derivatives[i])[qp];
      return res;

    case CHBulk<T>::Jacobian:
      res = this->_grad_phi[this->_j][qp] * (*_second_derivatives[0])[qp];
      for (unsigned int i = 0; i <= _nvar; ++i)
        res += this->_phi[this->_j][qp] * (*_grad_vars[i])[qp] * (*_third_derivatives[i])[qp];
      return res;
  }

  mooseError("Internal error");
}

template<typename T>
Real
CahnHilliardBase<T>::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  unsigned int cvar;
  if (!this->mapJvarToCvar(jvar, cvar))
    return 0.0;

  unsigned int & qp = this->_qp;
  RealGradient J =   this->_grad_u[qp] * this->_phi[this->_j][qp] * (*_third_derivatives[cvar+1])[qp]
                   + this->_grad_phi[this->_j][qp] * (*_second_derivatives[cvar+1])[qp];

  for (unsigned int i = 0; i < _nvar; ++i)
    J += this->_phi[this->_j][qp] * (*_grad_vars[i+1])[qp] * (*_third_cross_derivatives[i][cvar])[qp];

  return CHBulk<T>::computeQpOffDiagJacobian(jvar) + this->_M[qp] * this->_grad_test[this->_i][qp] * J;
}

#endif // CAHNHILLIARDBASE_H
