/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SPLITCHWRESBASE_H
#define SPLITCHWRESBASE_H

#include "Kernel.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

/**
 * SplitCHWresBase implements the residual for the chemical
 * potential in the split form of the Cahn-Hilliard
 * equation in a general way that can be templated to a scalar or
 * tensor mobility.
 */
template <typename T>
class SplitCHWResBase : public DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>
{
public:
  SplitCHWResBase(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  const MaterialPropertyName _mob_name;
  const MaterialProperty<T> & _mob;

  std::vector<const MaterialProperty<T> *> _dmobdarg;
};

template <typename T>
SplitCHWResBase<T>::SplitCHWResBase(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>(parameters),
    _mob_name(getParam<MaterialPropertyName>("mob_name")),
    _mob(getMaterialProperty<T>("mob_name"))
{
  // Get number of coupled variables
  unsigned int nvar = _coupled_moose_vars.size();

  // reserve space for derivatives
  _dmobdarg.resize(nvar);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < nvar; ++i)
    _dmobdarg[i] = &getMaterialPropertyDerivative<T>(_mob_name, _coupled_moose_vars[i]->name());
}

template <typename T>
InputParameters
SplitCHWResBase<T>::validParams()
{
  InputParameters params = ::validParams<Kernel>();
  params.addClassDescription(
      "Split formulation Cahn-Hilliard Kernel for the chemical potential variable");
  params.addParam<MaterialPropertyName>("mob_name", "mobtemp", "The mobility used with the kernel");
  params.addCoupledVar("args", "Vector of arguments of the mobility");
  return params;
}

template <typename T>
Real
SplitCHWResBase<T>::computeQpResidual()
{
  return _mob[_qp] * _grad_u[_qp] * _grad_test[_i][_qp];
}

template <typename T>
Real
SplitCHWResBase<T>::computeQpJacobian()
{
  return _mob[_qp] * _grad_phi[_j][_qp] * _grad_test[_i][_qp];
}

template <typename T>
Real
SplitCHWResBase<T>::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  return (*_dmobdarg[cvar])[_qp] * _phi[_j][_qp] * _grad_u[_qp] * _grad_test[_i][_qp];
}

#endif // SPLITCHWRESBASE_H
