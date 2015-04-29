/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#ifndef DERIVATIVEMATERIALINTERFACE_H
#define DERIVATIVEMATERIALINTERFACE_H

#include "Material.h"
#include "MaterialProperty.h"
#include "FEProblem.h"

#include <sstream>

/**
 * Helper function templates to set a variable to zero.
 * Specializations may have to be implemented (for examples see
 * RankTwoTensor, RankFourTensor, ElasticityTensorR4).
 */
template<typename T>
void mooseSetToZero(T & v)
{
  /**
   * The default for non-pointer types is to assign zero.
   * This should either do something sensible, or throw a compiler error.
   * Otherwise the T type is designed badly.
   */
  v = 0;
}
template<typename T>
void mooseSetToZero(T* &)
{
  mooseError("Cannot use pointer types for MaterialProperty derivatives.");
}

/**
 * Interface class ("Veneer") to provide generator methods for derivative
 * material property names, and guarded getMaterialPropertyPointer calls
 */
template<class T>
class DerivativeMaterialInterface : public T
{
public:
  DerivativeMaterialInterface(const std::string & name, InputParameters parameters);

  /**
   * Helper functions to generate the material property names for the
   * arbitrary derivatives.
   */
  const std::string propertyName(const std::string &base, const std::vector<std::string> &c) const;

  /**
   * Helper functions to generate the material property names for the
   * first derivatives.
   */
  const std::string propertyNameFirst(const std::string &base,
    const std::string &c1) const;

  /**
   * Helper functions to generate the material property names for the
   * second derivatives.
   */
  const std::string propertyNameSecond(const std::string &base,
    const std::string &c1, const std::string &c2) const;

  /**
   * Helper functions to generate the material property names for the
   * third derivatives.
   */
  const std::string propertyNameThird(const std::string &base,
    const std::string &c1, const std::string &c2, const std::string &c3) const;

  // Interface style (1)
  // return null pointers for non-existing material properties

  /**
   * Fetch a pointer to a material property if it exists, otherwise return null
   */
  template<typename U>
  MaterialProperty<U> * getMaterialPropertyPointer(const std::string & name);

  // Interface style (2)
  // return references to a zero material property for non-existing material properties

  /**
   * Fetch a material property if it exists, otherwise return a constant zero property
   */
  template<typename U>
  const MaterialProperty<U> & getDefaultMaterialProperty(const std::string & name);

  template<typename U>
  MaterialProperty<U> & declarePropertyDerivative(const std::string &base, const std::vector<std::string> &c);
  template<typename U>
  MaterialProperty<U> & declarePropertyDerivative(const std::string &base, const std::string &c1);
  template<typename U>
  MaterialProperty<U> & declarePropertyDerivative(const std::string &base, const std::string &c1, const std::string &c2);
  template<typename U>
  MaterialProperty<U> & declarePropertyDerivative(const std::string &base, const std::string &c1, const std::string &c2, const std::string &c3);

  template<typename U>
  const MaterialProperty<U> & getMaterialPropertyDerivative(const std::string &base, const std::vector<std::string> &c);
  template<typename U>
  const MaterialProperty<U> & getMaterialPropertyDerivative(const std::string &base, const std::string &c1);
  template<typename U>
  const MaterialProperty<U> & getMaterialPropertyDerivative(const std::string &base, const std::string &c1, const std::string &c2);
  template<typename U>
  const MaterialProperty<U> & getMaterialPropertyDerivative(const std::string &base, const std::string &c1, const std::string &c2, const std::string &c3);

private:
  FEProblem & _dmi_fe_problem;
};


template<class T>
DerivativeMaterialInterface<T>::DerivativeMaterialInterface(const std::string & name, InputParameters parameters) :
    T(name, parameters),
    _dmi_fe_problem(*parameters.getCheckedPointerParam<FEProblem *>("_fe_problem"))
{
}

template<class T>
const std::string
DerivativeMaterialInterface<T>::propertyName(const std::string &base,const std::vector<std::string> &c) const
{
  // to obtain well defined names we sort alphabetically
  std::vector<std::string> a(c);
  std::sort(a.begin(), a.end());

  // derivative order
  unsigned int order = a.size();
  if (order == 0)
    return base;

  // build the property name as a stringstream
  std::stringstream name;

  // build numerator
  name << 'd';
  if (order > 1)
    name << '^' << order;
  name << base << '/';

  // build denominator with 'pretty' names using exponents rather than repeat multiplication
  unsigned int exponent = 1;
  for (unsigned i = 1; i <= order; ++i)
  {
    if (i == order || a[i-1] != a[i])
    {
      name << 'd' << a[i-1];
      if (exponent > 1)
        name << '^' << exponent;
      exponent = 1;
    }
    else
      exponent++;
  }

  return name.str();
}

template<class T>
const std::string
DerivativeMaterialInterface<T>::propertyNameFirst(const std::string &base, const std::string &c1) const
{
  return "d" + base + "/d" + c1;
}

template<class T>
const std::string
DerivativeMaterialInterface<T>::propertyNameSecond(const std::string &base, const std::string &c1, const std::string &c2) const
{
  std::vector<std::string> c(2);
  c[0] = c1;
  c[1] = c2;
  return propertyName(base, c);
}

template<class T>
const std::string
DerivativeMaterialInterface<T>::propertyNameThird(const std::string &base, const std::string &c1, const std::string &c2, const std::string &c3) const
{
  std::vector<std::string> c(3);
  c[0] = c1;
  c[1] = c2;
  c[2] = c3;
  return propertyName(base, c);
}

template<class T>
template<typename U>
MaterialProperty<U> *
DerivativeMaterialInterface<T>::getMaterialPropertyPointer(const std::string & name)
{
  mooseDeprecated("getMaterialPropertyPointer is deprecated because it is construction order dependent. Use getDefaultMaterialProperty instead.");
  return this->template hasMaterialProperty<U>(name) ? &(this->template getMaterialProperty<U>(name)) : NULL;
}

template<>
template<typename U>
const MaterialProperty<U> &
DerivativeMaterialInterface<Material>::getDefaultMaterialProperty(const std::string & name)
{
  // if found return the requested property
  if (this->template hasMaterialProperty<U>(name))
    return this->template getMaterialProperty<U>(name);

  // declare this material property
  MaterialProperty<U> & preload_with_zero = this->template declareProperty<U>(name);

  // resize to accomodate maximum number of qpoints
  unsigned int nqp = _dmi_fe_problem.getMaxQps();
  preload_with_zero.resize(nqp);

  // set values for all qpoints to zero
  for (unsigned int qp = 0; qp < nqp; ++qp)
    mooseSetToZero<U>(preload_with_zero[qp]);

  return preload_with_zero;
}

template<class T>
template<typename U>
const MaterialProperty<U> &
DerivativeMaterialInterface<T>::getDefaultMaterialProperty(const std::string & name)
{
  static MaterialProperty<U> _zero;

  // if found return the requested property
  if (this->template hasMaterialProperty<U>(name))
    return this->template getMaterialProperty<U>(name);

  // make sure _zero is in a sane state
  unsigned int nqp = _dmi_fe_problem.getMaxQps();
  if (nqp > _zero.size())
  {
    // resize to accomodate maximum number of qpoints
    _zero.resize(nqp);

    // set values for all qpoints to zero
    for (unsigned int qp = 0; qp < nqp; ++qp)
      mooseSetToZero<U>(_zero[qp]);
  }

  // return a reference to a static zero property
  return _zero;
}

template<class T>
template<typename U>
MaterialProperty<U> &
DerivativeMaterialInterface<T>::declarePropertyDerivative(const std::string &base, const std::vector<std::string> &c)
{
  return this->template declareProperty<U>(propertyName(base, c));
}


template<class T>
template<typename U>
MaterialProperty<U> &
DerivativeMaterialInterface<T>::declarePropertyDerivative(const std::string &base, const std::string &c1)
{
  return this->template declareProperty<U>(propertyNameFirst(base, c1));
}

template<class T>
template<typename U>
MaterialProperty<U> &
DerivativeMaterialInterface<T>::declarePropertyDerivative(const std::string &base, const std::string &c1, const std::string &c2)
{
  return this->template declareProperty<U>(propertyNameSecond(base, c1, c2));
}

template<class T>
template<typename U>
MaterialProperty<U> &
DerivativeMaterialInterface<T>::declarePropertyDerivative(const std::string &base, const std::string &c1, const std::string &c2, const std::string &c3)
{
  return this->template declareProperty<U>(propertyNameThird(base, c1, c2, c3));
}


template<class T>
template<typename U>
const MaterialProperty<U> &
DerivativeMaterialInterface<T>::getMaterialPropertyDerivative(const std::string &base, const std::string &c1)
{
  return getDefaultMaterialProperty<U>(propertyNameFirst(base, c1));
}

template<class T>
template<typename U>
const MaterialProperty<U> &
DerivativeMaterialInterface<T>::getMaterialPropertyDerivative(const std::string &base, const std::string &c1, const std::string &c2)
{
  return getDefaultMaterialProperty<U>(propertyNameSecond(base, c1, c2));
}

template<class T>
template<typename U>
const MaterialProperty<U> &
DerivativeMaterialInterface<T>::getMaterialPropertyDerivative(const std::string &base, const std::string &c1, const std::string &c2, const std::string &c3)
{
  return getDefaultMaterialProperty<U>(propertyNameThird(base, c1, c2, c3));
}

#endif //DERIVATIVEMATERIALINTERFACE_H
