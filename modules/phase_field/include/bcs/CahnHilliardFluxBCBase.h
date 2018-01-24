//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CAHNHILLIARDFLUXBCBASE_H
#define CAHNHILLIARDFLUXBCBASE_H

#include "IntegratedBC.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

/**
 * Flux boundary condition base class for variable dependent mobilities. This
 * class must be templated on the mobility type, which can be either a scalar (Real)
 * or a tensor (RealValueTensor).
 */
template <typename T>
class CahnHilliardFluxBCBase
    : public DerivativeMaterialInterface<JvarMapIntegratedBCInterface<IntegratedBC>>
{
public:
  CahnHilliardFluxBCBase(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  virtual void initialSetup();

  const RealGradient _flux;

  const MaterialProperty<T> & _M;
  const MaterialProperty<T> & _dMdw;
  const MaterialProperty<T> & _d2Mdw2;
  std::vector<const MaterialProperty<T> *> _dMdarg;
  std::vector<const MaterialProperty<T> *> _d2Mdargdw;

  const Real _penalty;
};

template <typename T>
CahnHilliardFluxBCBase<T>::CahnHilliardFluxBCBase(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapIntegratedBCInterface<IntegratedBC>>(parameters),
    _flux(getParam<RealGradient>("flux")),
    _M(getMaterialProperty<T>("mob_name")),
    _dMdw(getMaterialPropertyDerivative<T>("mob_name", _var.name())),
    _d2Mdw2(getMaterialPropertyDerivative<T>("mob_name", _var.name(), _var.name())),
    _penalty(getParam<Real>("penalty"))
{
  // Get number of coupled variables
  unsigned int nvar = _coupled_moose_vars.size();

  // reserve space for derivatives
  _dMdarg.resize(nvar);
  _d2Mdargdw.resize(nvar);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < nvar; ++i)
  {
    _dMdarg[i] = &getMaterialPropertyDerivative<T>("mob_name", _coupled_moose_vars[i]->name());
    _d2Mdargdw[i] =
        &getMaterialPropertyDerivative<T>("mob_name", _coupled_moose_vars[i]->name(), _var.name());
  }
}

template <typename T>
InputParameters
CahnHilliardFluxBCBase<T>::validParams()
{
  InputParameters params = ::validParams<IntegratedBC>();
  params.addClassDescription("Cahn-Hilliard base Kernel");
  params.addParam<RealGradient>("flux", "The flux set at the boundary");
  params.addParam<MaterialPropertyName>("mob_name", "M", "The mobility used with the kernel");
  params.addCoupledVar("args", "Vector of arguments of the mobility");
  params.addParam<Real>(
      "penalty", 1000.0, "Penalty factor for the weak enforcement of the boundary condition.");
  return params;
}

template <typename T>
void
CahnHilliardFluxBCBase<T>::initialSetup()
{
  validateNonlinearCoupling<Real>("mob_name");
}

template <typename T>
Real
CahnHilliardFluxBCBase<T>::computeQpResidual()
{
  return _penalty * ((_M[_qp] * _grad_u[_qp] - _flux) * _normals[_qp]) *
         (_M[_qp] * _normals[_qp] * _grad_test[_i][_qp] +
          _dMdw[_qp] * _grad_u[_qp] * _normals[_qp] * _test[_i][_qp]);
}

template <typename T>
Real
CahnHilliardFluxBCBase<T>::computeQpJacobian()
{
  return _penalty * ((_M[_qp] * _grad_phi[_j][_qp] + _phi[_j][_qp] * _dMdw[_qp] * _grad_u[_qp]) *
                     _normals[_qp]) *
             (_M[_qp] * _normals[_qp] * _grad_test[_i][_qp] +
              _dMdw[_qp] * _grad_u[_qp] * _normals[_qp] * _test[_i][_qp]) +
         _penalty * ((_M[_qp] * _grad_u[_qp] - _flux) * _normals[_qp]) *
             (_dMdw[_qp] * _normals[_qp] * _grad_phi[_j][_qp] * _test[_i][_qp] +
              _dMdw[_qp] * _phi[_j][_qp] * _grad_test[_i][_qp] * _normals[_qp] +
              _d2Mdw2[_qp] * _phi[_j][_qp] * _grad_u[_qp] * _normals[_qp] * _test[_i][_qp]);
}

template <typename T>
Real
CahnHilliardFluxBCBase<T>::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);
  return _penalty * (_phi[_j][_qp] * (*_dMdarg[cvar])[_qp] * _grad_u[_qp] * _normals[_qp]) *
             (_M[_qp] * _normals[_qp] * _grad_test[_i][_qp] +
              _dMdw[_qp] * _grad_u[_qp] * _normals[_qp] * _test[_i][_qp]) +
         _penalty * ((_M[_qp] * _grad_u[_qp] - _flux) * _normals[_qp]) *
             (_dMdw[_qp] * _phi[_j][_qp] * _grad_test[_i][_qp] * _normals[_qp] +
              (*_d2Mdargdw[cvar])[_qp] * _phi[_j][_qp] * _grad_u[_qp] * _normals[_qp] *
                  _test[_i][_qp]);
}

#endif // CAHNHILLIARDFLUXBCBASE_H
