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
   *
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
  const MaterialProperty<U> & getMaterialPropertyDerivativeRelap(const std::string & base,
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
  const MaterialProperty<U> & getMaterialPropertyDerivativeRelapPhase(const std::string & base,
                                                                      bool property_is_liquid,
                                                                      const std::string & var_name,
                                                                      bool var_is_liquid,
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
  return this->template declarePropertyDerivative<U>(base, this->getVar(var_name, i)->name());
}

template <class T>
template <typename U>
const MaterialProperty<U> &
DerivativeMaterialInterfaceRelap<T>::getMaterialPropertyDerivativeRelap(
    const std::string & base, const std::string & var_name, const unsigned int i)
{
  // get the base property name
  const std::string prop_name = this->deducePropertyName(base);

  // get the name of the variable which derivative is respect to
  const std::string der_var_name = this->getVar(var_name, i)->name();

  return this->template getMaterialPropertyByName<U>(
      this->propertyNameFirst(prop_name, der_var_name));
}

template <class T>
template <typename U>
const MaterialProperty<U> &
DerivativeMaterialInterfaceRelap<T>::getMaterialPropertyDerivativeRelapPhase(
    const std::string & base,
    bool property_is_liquid,
    const std::string & var_name,
    bool var_is_liquid,
    const unsigned int i)
{
  if (property_is_liquid == var_is_liquid)
  {
    return getMaterialPropertyDerivativeRelap<U>(base, var_name, i);
  }
  else
  {
    const std::string prop_name = this->deducePropertyName(base);
    const std::string der_var_name = this->getVar(var_name, i)->name();
    const std::string der_prop_name = this->propertyNameFirst(prop_name, der_var_name);
    return this->template getZeroMaterialProperty<U>(der_prop_name);
  }
}

#endif // DERIVATIVEMATERIALINTERFACERELAP_H
