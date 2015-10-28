/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MATDIFFUSIONBASE_H
#define MATDIFFUSIONBASE_H

#include "Kernel.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

/**
 * This class template implements a diffusion kernel with a mobility that can vary
 * spatially and can depend on variables in the simulation. Two classes are derived from
 * this template, MatDiffusion for isotropic diffusion and MatAnisoDiffusion for
 * anisotropic diffusion.
 *
 * \tparam T Type of the diffusion coefficient parameter. This can be Real for
 *           isotropic diffusion or RealTensorValue for the general anisotropic case.
 */
template<typename T>
class MatDiffusionBase : public DerivativeMaterialInterface<JvarMapInterface<Kernel> >
{
public:
  MatDiffusionBase(const InputParameters & parameters);

  virtual void initialSetup();

  /// in class templates this function has to be a static member
  static InputParameters validParams();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// diffusion coefficient
  const MaterialProperty<T> & _D;

  /// diffusion coefficient derivative w.r.t. the kernel variable
  const MaterialProperty<T> & _dDdc;

  /// diffusion coefficient derivatives w.r.t. coupled variables
  std::vector<const MaterialProperty<T> *> _dDdarg;
};

template<typename T>
InputParameters
MatDiffusionBase<T>::validParams()
{
  InputParameters params = ::validParams<Kernel>();
  params.addParam<MaterialPropertyName>("D_name", "D", "The name of the diffusivity");
  params.addCoupledVar("args", "Vector of arguments of the diffusivity");
  return params;
}

template<typename T>
MatDiffusionBase<T>::MatDiffusionBase(const InputParameters & parameters) :
    DerivativeMaterialInterface<JvarMapInterface<Kernel> >(parameters),
    _D(getMaterialProperty<T>("D_name")),
    _dDdc(getMaterialPropertyDerivative<T>("D_name", _var.name())),
    _dDdarg(_coupled_moose_vars.size())
{
  // fetch derivatives
  for (unsigned int i = 0; i < _dDdarg.size(); ++i)
    _dDdarg[i] = &getMaterialPropertyDerivative<T>("D_name", _coupled_moose_vars[i]->name());
}

template<typename T>
void
MatDiffusionBase<T>::initialSetup()
{
  validateNonlinearCoupling<Real>("D_name");
}

template<typename T>
Real
MatDiffusionBase<T>::computeQpResidual()
{
  return _D[_qp] * _grad_u[_qp] * _grad_test[_i][_qp];
}

template<typename T>
Real
MatDiffusionBase<T>::computeQpJacobian()
{
  return (_D[_qp] * _grad_phi[_j][_qp] + _phi[_j][_qp] * _dDdc[_qp] * _grad_u[_qp]) * _grad_test[_i][_qp];
}

template<typename T>
Real
MatDiffusionBase<T>::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  unsigned int cvar;
  if (!mapJvarToCvar(jvar, cvar))
    return 0.0;

  return (*_dDdarg[cvar])[_qp] * _phi[_j][_qp] * _grad_u[_qp] * _grad_test[_i][_qp];
}

#endif //MATDIFFUSIONBASE_H
