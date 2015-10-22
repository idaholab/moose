/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MATDIFFUSIONBASE_H
#define MATDIFFUSIONBASE_H

#include "DerivativeMaterialInterface.h"
#include "Kernel.h"

/**
 * This class template implements a diffusion kernel with a mobility that can vary
 * spatially and can depend on variables in the simulation. Two classes are derived from
 * this template, MatDiffusion for isotropic diffusion and MatAnisoDiffusion for
 * anisotropic diffusion.
 *
 * \tparam T Type of the diffusion coefficient parameter. This can be Real for
 *           isotriopc diffusion or RealTensorValue for the general anisotropic case.
 */
template<typename T>
class MatDiffusionBase : public DerivativeMaterialInterface<Kernel>
{
public:
  MatDiffusionBase(const InputParameters & parameters);

  /// in class templates this function has to be a static member
  static InputParameters validParams();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  /// diffusion coefficient
  const MaterialProperty<T> & _D;
};

template<typename T>
InputParameters
MatDiffusionBase<T>::validParams()
{
  InputParameters params = ::validParams<Kernel>();
  params.addParam<MaterialPropertyName>("D_name", "D", "The name of the diffusivity");
  return params;
}

template<typename T>
MatDiffusionBase<T>::MatDiffusionBase(const InputParameters & parameters) :
    DerivativeMaterialInterface<Kernel>(parameters),
    _D(getMaterialProperty<T>("D_name"))
{
}

template<typename T>
Real
MatDiffusionBase<T>::computeQpResidual()
{
  return _D[_qp] * _grad_test[_i][_qp] * _grad_u[_qp];
}

template<typename T>
Real
MatDiffusionBase<T>::computeQpJacobian()
{
  return _D[_qp] * _grad_test[_i][_qp] * _grad_phi[_j][_qp];
}

#endif //MATDIFFUSIONBASE_H
