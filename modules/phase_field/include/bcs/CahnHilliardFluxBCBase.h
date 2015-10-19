/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CAHNHILLIARDFLUXBCBASE_H
#define CAHNHILLIARDFLUXBCBASE_H

#include "IntegratedBC.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

/**
 * Flux boundary condition base class for variable dependent mobilities. This
 * Class must be templated on teh mobility type, which can be either a scalar (Real)
 * or a tensor (RealValueTensor).
 */
template<typename T>
class CahnHilliardFluxBCBase : public DerivativeMaterialInterface<JvarMapInterface<IntegratedBC> >
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
  const MaterialProperty<T> & _dMdc;
  std::vector<const MaterialProperty<T> *> _dMdarg;
};

template<typename T>
CahnHilliardFluxBCBase<T>::CahnHilliardFluxBCBase(const InputParameters & parameters) :
    DerivativeMaterialInterface<JvarMapInterface<IntegratedBC> >(parameters),
    _flux(getParam<RealGradient>("flux")),
    _M(getMaterialProperty<T>("mob_name")),
    _dMdc(getMaterialPropertyDerivative<T>("mob_name", _var.name()))
{
  // Get number of coupled variables
  unsigned int nvar = _coupled_moose_vars.size();

  // reserve space for derivatives
  _dMdarg.resize(nvar);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < nvar; ++i)
    _dMdarg[i] = &getMaterialPropertyDerivative<T>("mob_name", _coupled_moose_vars[i]->name());
}

template<typename T>
InputParameters
CahnHilliardFluxBCBase<T>::validParams()
{
  InputParameters params = ::validParams<IntegratedBC>();
  params.addClassDescription("Cahn-Hilliard base Kernel");
  params.addParam<RealGradient>("flux", "The flux set at the boundary");
  params.addParam<MaterialPropertyName>("mob_name", "M", "The mobility used with the kernel");
  params.addCoupledVar("args", "Vector of arguments to mobility");
  return params;
}

template<typename T>
void
CahnHilliardFluxBCBase<T>::initialSetup()
{
  validateNonlinearCoupling<Real>("mob_name");
}

template<typename T>
Real
CahnHilliardFluxBCBase<T>::computeQpResidual()
{
  return (_flux - _M[_qp] * _grad_u[_qp]) * _normals[_qp] * _test[_i][_qp];
}

template<typename T>
Real
CahnHilliardFluxBCBase<T>::computeQpJacobian()
{
  return (_flux - (_M[_qp] * _grad_phi[_j][_qp] + _phi[_j][_qp] * _dMdc[_qp] * _grad_u[_qp]))  * _normals[_qp] * _test[_i][_qp];
}

template<typename T>
Real
CahnHilliardFluxBCBase<T>::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  unsigned int cvar;
  if (!mapJvarToCvar(jvar, cvar))
    return 0.0;

  return (_flux - _phi[_j][_qp] * (*_dMdarg[cvar])[_qp] * _grad_u[_qp])  * _normals[_qp] * _test[_i][_qp];
}

#endif //CAHNHILLIARDFLUXBCBASE_H
