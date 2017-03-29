#ifndef DERIVATIVEMATERIALINTERFACERELAP_H
#define DERIVATIVEMATERIALINTERFACERELAP_H

#include "DerivativeMaterialInterface.h"

template <class T>
class DerivativeMaterialInterfaceRelap : public DerivativeMaterialInterface<T>
{
public:
  DerivativeMaterialInterfaceRelap(const InputParameters & parameters);

protected:
  /**
   * Method for declaring derivative material properties
   * @tparam U The material property type
   * @param base The name of the property to take the derivative of
   * @param var_name The name of the coupled variable vector containing the variable to take the
   * derivative with respect to
   * @param i The index of the variable to take the derivative with respect to within the coupled
   * variable vector
   */
  template <typename U>
  MaterialProperty<U> & declarePropertyDerivativeRelap(const std::string & base,
                                                       const std::string & var_name,
                                                       const unsigned int i = 0);

  /**
   * Method for retreiving derivative material properties
   * @tparam U The material property type
   * @param base The name of the property to take the derivative of
   * @param var_name The name of the coupled variable vector containing the variable to take the
   * derivative with respect to
   * @param i The index of the variable to take the derivative with respect to within the coupled
   * variable vector
   */
  template <typename U>
  const MaterialProperty<U> & getMaterialPropertyDerivativeRelap(const std::string & base,
                                                                 const std::string & var_name,
                                                                 const unsigned int i = 0);
};

template <class T>
DerivativeMaterialInterfaceRelap<T>::DerivativeMaterialInterfaceRelap(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<T>(parameters)
{
}

template <class T>
template <typename U>
MaterialProperty<U> &
DerivativeMaterialInterfaceRelap<T>::declarePropertyDerivativeRelap(const std::string & base,
                                                                    const std::string & var_name,
                                                                    const unsigned int i)
{
  return this->template declarePropertyDerivative<U>(base,
                                                     this->template getVar(var_name, i)->name());
}

template <class T>
template <typename U>
const MaterialProperty<U> &
DerivativeMaterialInterfaceRelap<T>::getMaterialPropertyDerivativeRelap(
    const std::string & base, const std::string & var_name, const unsigned int i)
{
  return this->template getMaterialPropertyDerivative<U>(
      base, this->template getVar(var_name, i)->name());
}

#endif // DERIVATIVEMATERIALINTERFACERELAP_H
