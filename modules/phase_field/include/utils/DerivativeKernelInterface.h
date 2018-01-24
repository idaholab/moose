//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
