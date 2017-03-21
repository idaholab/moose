/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef DERIVATIVEKERNELINTERFACE_H
#define DERIVATIVEKERNELINTERFACE_H

#include "DerivativeMaterialInterface.h"

/**
 * Interface class ("Veneer") to provide generator methods for derivative
 * material property names, and guarded getMaterialPropertyPointer calls
 */
template <class T>
class DerivativeKernelInterface : public DerivativeMaterialInterface<T>
{
public:
  DerivativeKernelInterface(const InputParameters & parameters);

  /// as partial template specialization is not allowed in C++ we have to implement this as a static method
  static InputParameters validParams();

protected:
  unsigned int _nvar;
  std::string _F_name;
};

template <class T>
DerivativeKernelInterface<T>::DerivativeKernelInterface(const InputParameters & parameters)
  : DerivativeMaterialInterface<T>(parameters),
    _nvar(this->_coupled_moose_vars.size()),
    _F_name(this->template getParam<std::string>("f_name"))
{
}

template <class T>
InputParameters
DerivativeKernelInterface<T>::validParams()
{
  InputParameters params = ::validParams<T>();
  params.addRequiredParam<std::string>(
      "f_name", "Base name of the free energy function F defined in a DerivativeParsedMaterial");
  return params;
}

#endif // DERIVATIVEKERNELINTERFACE_H
