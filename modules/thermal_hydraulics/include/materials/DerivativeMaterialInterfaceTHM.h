//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DerivativeMaterialInterface.h"

template <class T>
class DerivativeMaterialInterfaceTHM : public DerivativeMaterialInterface<T>
{
public:
  DerivativeMaterialInterfaceTHM(const InputParameters & parameters);

protected:
  /**
   * Method for declaring derivative material properties
   *
   * @tparam U The material property type
   * @param base The name of the property to take the derivative of
   * @param var_name The name of the coupled variable vector containing the variable to take the
   * derivative with respect to
   * @param i The index of the variable to take the derivative with respect to within the coupled
   * variable vector
   */
  template <typename U>
  MaterialProperty<U> & declarePropertyDerivativeTHM(const std::string & base,
                                                     const std::string & var_name,
                                                     const unsigned int i = 0);

  /**
   * Method for retrieving derivative material properties
   *
   * @tparam U The material property type
   * @param base The name of the property to take the derivative of
   * @param var_name The name of the coupled variable vector containing the variable to take the
   * derivative with respect to
   * @param i The index of the variable to take the derivative with respect to within the coupled
   * variable vector
   */
  template <typename U>
  const MaterialProperty<U> & getMaterialPropertyDerivativeTHM(const std::string & base,
                                                               const std::string & var_name,
                                                               const unsigned int i = 0);

  /**
   * Method for retrieving derivative material properties corresponding to phase-dependent
   * derivatives.
   *
   * The intended behavior of this function is as follows. If the material property
   * and derivative variable correspond to the same phase, then the material property
   * is retrieved as usual. Else, a zero-valued material property is returned, as
   * it is assumed that the material property depends only upon the variables
   * corresponding to its phase.
   *
   * @tparam U The material property type
   * @param base The name of the property to take the derivative of
   * @param property_is_liquid  Does the property correspond to the liquid phase?
   * @param var_name The name of the coupled variable vector containing the variable to take the
   *   derivative with respect to
   * @param var_is_liquid  Does the derivative variable correspond to the liquid phase?
   * @param i The index of the variable to take the derivative with respect to within the coupled
   *   variable vector
   */
  template <typename U>
  const MaterialProperty<U> & getMaterialPropertyDerivativeTHMPhase(const std::string & base,
                                                                    bool property_is_liquid,
                                                                    const std::string & var_name,
                                                                    bool var_is_liquid,
                                                                    const unsigned int i = 0);
};

template <class T>
DerivativeMaterialInterfaceTHM<T>::DerivativeMaterialInterfaceTHM(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<T>(parameters)
{
}

template <class T>
template <typename U>
MaterialProperty<U> &
DerivativeMaterialInterfaceTHM<T>::declarePropertyDerivativeTHM(const std::string & base,
                                                                const std::string & var_name,
                                                                const unsigned int i)
{
  return this->template declarePropertyDerivative<U>(base, this->coupledName(var_name, i));
}

template <class T>
template <typename U>
const MaterialProperty<U> &
DerivativeMaterialInterfaceTHM<T>::getMaterialPropertyDerivativeTHM(const std::string & base,
                                                                    const std::string & var_name,
                                                                    const unsigned int i)
{
  // get the base property name
  const std::string prop_name = this->deducePropertyName(base);

  // get the name of the variable which derivative is respect to
  const std::string der_var_name = this->coupledName(var_name, i);

  return this->template getMaterialPropertyByName<U>(
      this->derivativePropertyNameFirst(prop_name, der_var_name));
}

template <class T>
template <typename U>
const MaterialProperty<U> &
DerivativeMaterialInterfaceTHM<T>::getMaterialPropertyDerivativeTHMPhase(
    const std::string & base,
    bool property_is_liquid,
    const std::string & var_name,
    bool var_is_liquid,
    const unsigned int i)
{
  if (property_is_liquid == var_is_liquid)
  {
    return getMaterialPropertyDerivativeTHM<U>(base, var_name, i);
  }
  else
  {
    const std::string prop_name = this->deducePropertyName(base);
    const std::string der_var_name = this->coupledName(var_name, i);
    const std::string der_prop_name = this->derivativePropertyNameFirst(prop_name, der_var_name);
    return this->template getZeroMaterialProperty<U>(der_prop_name);
  }
}
