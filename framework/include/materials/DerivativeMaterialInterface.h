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
#include "BlockRestrictable.h"
#include "BoundaryRestrictable.h"
#include "DerivativeMaterialPropertyNameInterface.h"

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
 * material property names
 */
template<class T>
class DerivativeMaterialInterface :
  public T,
  public DerivativeMaterialPropertyNameInterface
{
public:
  DerivativeMaterialInterface(const std::string & name, InputParameters parameters);

  /**
   * Fetch a material property if it exists, otherwise return getZeroMaterialProperty.
   * @param name The input parameter key of type MaterialPropertyName
   */
  template<typename U>
  const MaterialProperty<U> & getDefaultMaterialProperty(const std::string & name);

  /// Fetch a material property by name if it exists, otherwise return getZeroMaterialProperty
  template<typename U>
  const MaterialProperty<U> & getDefaultMaterialPropertyByName(const std::string & name);

  ///@{
  /**
   * Methods for declaring derivative material properties
   * @tparam U The material property type
   * @param base The name of the property to take the derivative of
   * @param c The variable(s) to take the derivatives with respect to
   */
  template<typename U>
  MaterialProperty<U> & declarePropertyDerivative(const std::string &base, const std::vector<VariableName> &c);
  template<typename U>
  MaterialProperty<U> & declarePropertyDerivative(const std::string &base, const VariableName &c1);
  template<typename U>
  MaterialProperty<U> & declarePropertyDerivative(const std::string &base, const VariableName &c1, const VariableName &c2);
  template<typename U>
  MaterialProperty<U> & declarePropertyDerivative(const std::string &base, const VariableName &c1, const VariableName &c2, const VariableName &c3);
  ///@}

  ///@{
  /**
   * Methods for retreiving derivative material properties
   * @tparam U The material property type
   * @param base The name of the property to take the derivative of
   * @param c The variable(s) to take the derivatives with respect to
   */
  template<typename U>
  const MaterialProperty<U> & getMaterialPropertyDerivative(const std::string &base, const std::vector<VariableName> &c);
  template<typename U>
  const MaterialProperty<U> & getMaterialPropertyDerivative(const std::string &base, const VariableName &c1);
  template<typename U>
  const MaterialProperty<U> & getMaterialPropertyDerivative(const std::string &base, const VariableName &c1, const VariableName &c2);
  template<typename U>
  const MaterialProperty<U> & getMaterialPropertyDerivative(const std::string &base, const VariableName &c1, const VariableName &c2, const VariableName &c3);
  ///@}

  ///@{
  /**
   * Methods for retreiving derivative material properties
   * @tparam U The material property type
   * @param base The name of the property to take the derivative of
   * @param c The variable(s) to take the derivatives with respect to
   */
  template<typename U>
  const MaterialProperty<U> & getMaterialPropertyDerivativeByName(const MaterialPropertyName &base, const std::vector<VariableName> &c);
  template<typename U>
  const MaterialProperty<U> & getMaterialPropertyDerivativeByName(const MaterialPropertyName &base, const VariableName &c1);
  template<typename U>
  const MaterialProperty<U> & getMaterialPropertyDerivativeByName(const MaterialPropertyName &base, const VariableName &c1, const VariableName &c2);
  template<typename U>
  const MaterialProperty<U> & getMaterialPropertyDerivativeByName(const MaterialPropertyName &base, const VariableName &c1, const VariableName &c2, const VariableName &c3);
  ///@}

private:
  /// Return a constant zero property
  template<typename U>
  const MaterialProperty<U> & getZeroMaterialProperty(const std::string & prop_name);

  /// Check if a material property is present with the applicable restrictions
  template<typename U>
  bool haveMaterialProperty(const std::string & prop_name);

  /// Reference to FEProblem
  FEProblem & _dmi_fe_problem;

  /// Reference to this objects MaterialData object
  MaterialData & _dmi_material_data;
};


template<class T>
DerivativeMaterialInterface<T>::DerivativeMaterialInterface(const std::string & name, InputParameters parameters) :
    T(name, parameters),
    _dmi_fe_problem(*parameters.getCheckedPointerParam<FEProblem *>("_fe_problem")),
    _dmi_material_data(*parameters.getCheckedPointerParam<MaterialData *>("_material_data"))
{
}

template<>
template<typename U>
const MaterialProperty<U> &
DerivativeMaterialInterface<Material>::getZeroMaterialProperty(const std::string & prop_name)
{
  // declare this material property
  MaterialProperty<U> & preload_with_zero = this->template declareProperty<U>(prop_name);

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
DerivativeMaterialInterface<T>::getZeroMaterialProperty(const std::string & /* prop_name */)
{
  static MaterialProperty<U> _zero;

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

template<>
template<typename U>
bool
DerivativeMaterialInterface<Material>::haveMaterialProperty(const std::string & prop_name)
{
  return ((this->boundaryRestricted() && this->template hasBoundaryMaterialProperty<U>(prop_name)) ||
         (this->template hasBlockMaterialProperty<U>(prop_name)));
}

template<class T>
template<typename U>
bool
DerivativeMaterialInterface<T>::haveMaterialProperty(const std::string & prop_name)
{
  // Call the correct method to test for material property declarations
  BlockRestrictable * blk = dynamic_cast<BlockRestrictable *>(this);
  BoundaryRestrictable * bnd = dynamic_cast<BoundaryRestrictable *>(this);
  return ((bnd && bnd->boundaryRestricted() && bnd->template hasBoundaryMaterialProperty<U>(prop_name)) ||
         (blk && blk->template hasBlockMaterialProperty<U>(prop_name)) ||
         (this->template hasMaterialProperty<U>(prop_name)));
}

template<class T>
template<typename U>
const MaterialProperty<U> &
DerivativeMaterialInterface<T>::getDefaultMaterialProperty(const std::string & name)
{
  // Check if it's just a constant
  const MaterialProperty<U> * default_property = this->template defaultMaterialProperty<U>(name);
  if (default_property)
    return *default_property;

  // if found return the requested property
  MaterialPropertyName prop_name = this->deducePropertyName(name);
  if (haveMaterialProperty<U>(prop_name))
    return this->template getMaterialProperty<U>(prop_name);

  return getZeroMaterialProperty<U>(prop_name);
}

template<class T>
template<typename U>
const MaterialProperty<U> &
DerivativeMaterialInterface<T>::getDefaultMaterialPropertyByName(const std::string & prop_name)
{
  // if found return the requested property
  if (haveMaterialProperty<U>(prop_name))
    return this->template getMaterialPropertyByName<U>(prop_name);

  return getZeroMaterialProperty<U>(prop_name);
}


template<class T>
template<typename U>
MaterialProperty<U> &
DerivativeMaterialInterface<T>::declarePropertyDerivative(const std::string &base, const std::vector<VariableName> &c)
{
  return this->template declareProperty<U>(propertyName(base, c));
}

template<class T>
template<typename U>
MaterialProperty<U> &
DerivativeMaterialInterface<T>::declarePropertyDerivative(const std::string &base, const VariableName &c1)
{
  return this->template declareProperty<U>(propertyNameFirst(base, c1));
}

template<class T>
template<typename U>
MaterialProperty<U> &
DerivativeMaterialInterface<T>::declarePropertyDerivative(const std::string &base, const VariableName &c1, const VariableName &c2)
{
  return this->template declareProperty<U>(propertyNameSecond(base, c1, c2));
}

template<class T>
template<typename U>
MaterialProperty<U> &
DerivativeMaterialInterface<T>::declarePropertyDerivative(const std::string &base, const VariableName &c1, const VariableName &c2, const VariableName &c3)
{
  return this->template declareProperty<U>(propertyNameThird(base, c1, c2, c3));
}


template<class T>
template<typename U>
const MaterialProperty<U> &
DerivativeMaterialInterface<T>::getMaterialPropertyDerivative(const std::string &base, const std::vector<VariableName> &c)
{
  return getDefaultMaterialPropertyByName<U>(propertyName(this->deducePropertyName(base), c));
}

template<class T>
template<typename U>
const MaterialProperty<U> &
DerivativeMaterialInterface<T>::getMaterialPropertyDerivative(const std::string &base, const VariableName &c1)
{
  return getDefaultMaterialPropertyByName<U>(propertyNameFirst(this->deducePropertyName(base), c1));
}

template<class T>
template<typename U>
const MaterialProperty<U> &
DerivativeMaterialInterface<T>::getMaterialPropertyDerivative(const std::string &base, const VariableName &c1, const VariableName &c2)
{
  return getDefaultMaterialPropertyByName<U>(propertyNameSecond(this->deducePropertyName(base), c1, c2));
}

template<class T>
template<typename U>
const MaterialProperty<U> &
DerivativeMaterialInterface<T>::getMaterialPropertyDerivative(const std::string &base, const VariableName &c1, const VariableName &c2, const VariableName &c3)
{
  return getDefaultMaterialPropertyByName<U>(propertyNameThird(this->deducePropertyName(base), c1, c2, c3));
}


template<class T>
template<typename U>
const MaterialProperty<U> &
DerivativeMaterialInterface<T>::getMaterialPropertyDerivativeByName(const MaterialPropertyName &base, const std::vector<VariableName> &c)
{
  return getDefaultMaterialProperty<U>(propertyName(base, c));
}

template<class T>
template<typename U>
const MaterialProperty<U> &
DerivativeMaterialInterface<T>::getMaterialPropertyDerivativeByName(const MaterialPropertyName &base, const VariableName &c1)
{
  return getDefaultMaterialProperty<U>(propertyNameFirst(base, c1));
}

template<class T>
template<typename U>
const MaterialProperty<U> &
DerivativeMaterialInterface<T>::getMaterialPropertyDerivativeByName(const MaterialPropertyName &base, const VariableName &c1, const VariableName &c2)
{
  return getDefaultMaterialProperty<U>(propertyNameSecond(base, c1, c2));
}

template<class T>
template<typename U>
const MaterialProperty<U> &
DerivativeMaterialInterface<T>::getMaterialPropertyDerivativeByName(const MaterialPropertyName &base, const VariableName &c1, const VariableName &c2, const VariableName &c3)
{
  return getDefaultMaterialProperty<U>(propertyNameThird(base, c1, c2, c3));
}

#endif //DERIVATIVEMATERIALINTERFACE_H
