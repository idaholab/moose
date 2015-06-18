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
 * material property names, and guarded getMaterialPropertyPointer calls
 */
template<class T>
class DerivativeMaterialInterface :
  public T,
  public DerivativeMaterialPropertyNameInterface
{
public:
  DerivativeMaterialInterface(const std::string & name, InputParameters parameters);

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

  ///@{
  /**
   * Methods for declaring derivative material properties
   * @tparam U The material property type
   * @param base The name of the property to take the derivative of
   * @param c The variable(s) to take the derivatives with respect to
   */
  template<typename U>
  MaterialProperty<U> & declarePropertyDerivative(const std::string &base, const std::vector<std::string> &c);
  template<typename U>
  MaterialProperty<U> & declarePropertyDerivative(const std::string &base, const std::string &c1);
  template<typename U>
  MaterialProperty<U> & declarePropertyDerivative(const std::string &base, const std::string &c1, const std::string &c2);
  template<typename U>
  MaterialProperty<U> & declarePropertyDerivative(const std::string &base, const std::string &c1, const std::string &c2, const std::string &c3);
  ///@}

  ///@{
  /**
   * Methods for retreiving derivative material properties
   * @tparam U The material property type
   * @param base The name of the property to take the derivative of
   * @param c The variable(s) to take the derivatives with respect to
   */
  template<typename U>
  const MaterialProperty<U> & getMaterialPropertyDerivative(const std::string &base, const std::vector<std::string> &c);
  template<typename U>
  const MaterialProperty<U> & getMaterialPropertyDerivative(const std::string &base, const std::string &c1);
  template<typename U>
  const MaterialProperty<U> & getMaterialPropertyDerivative(const std::string &base, const std::string &c1, const std::string &c2);
  template<typename U>
  const MaterialProperty<U> & getMaterialPropertyDerivative(const std::string &base, const std::string &c1, const std::string &c2, const std::string &c3);
  ///@}

private:
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
DerivativeMaterialInterface<Material>::getDefaultMaterialProperty(const std::string & prop_name)
{

  // if found return the requested property
  if (this->boundaryRestricted() && this->template hasBoundaryMaterialProperty<U>(prop_name))
    return this->template getMaterialProperty<U>(prop_name);

  else if (this->template hasBlockMaterialProperty<U>(prop_name))
    return this->template getMaterialProperty<U>(prop_name);

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
DerivativeMaterialInterface<T>::getDefaultMaterialProperty(const std::string & prop_name)
{
  static MaterialProperty<U> _zero;

  // Call the correct method to test for material property declarations
  BlockRestrictable * blk = dynamic_cast<BlockRestrictable *>(this);
  BoundaryRestrictable * bnd = dynamic_cast<BoundaryRestrictable *>(this);
  if (bnd && bnd->boundaryRestricted() && bnd->template hasBoundaryMaterialProperty<U>(prop_name))
    return this->template getMaterialProperty<U>(prop_name);

  else if (blk && blk->template hasBlockMaterialProperty<U>(prop_name))
    return this->template getMaterialProperty<U>(prop_name);

  else if (this->template hasMaterialProperty<U>(prop_name))
    return this->template getMaterialProperty<U>(prop_name);

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
