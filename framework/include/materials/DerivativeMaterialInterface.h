//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxiliarySystem.h"
#include "BlockRestrictable.h"
#include "BoundaryRestrictable.h"
#include "DerivativeMaterialPropertyNameInterface.h"
#include "KernelBase.h"
#include "BoundaryCondition.h"
#include "Material.h"
#include "MooseVariableFE.h"
#include "NonlinearSystem.h"

// Forward declarations
class FEProblemBase;
template <typename>
class MaterialProperty;

/**
 * Interface class ("Veneer") to provide generator methods for derivative
 * material property names
 */
template <class T>
class DerivativeMaterialInterface : public T, public DerivativeMaterialPropertyNameInterface
{
public:
  typedef DerivativeMaterialPropertyNameInterface::SymbolName SymbolName;

  DerivativeMaterialInterface(const InputParameters & parameters);

  /**
   * Fetch a material property if it exists, otherwise return getZeroMaterialProperty.
   * @param name The input parameter key of type MaterialPropertyName
   */
  template <typename U, bool is_ad = false>
  const GenericMaterialProperty<U, is_ad> & getDefaultMaterialProperty(const std::string & name);

  /// Fetch a material property by name if it exists, otherwise return getZeroMaterialProperty
  template <typename U, bool is_ad = false>
  const GenericMaterialProperty<U, is_ad> &
  getDefaultMaterialPropertyByName(const std::string & name);

  ///@{
  /**
   * Methods for declaring derivative material properties
   * @tparam U The material property type
   * @param base The name of the property to take the derivative of
   * @param c The variable(s) to take the derivatives with respect to
   */
  template <typename U, bool is_ad = false>
  GenericMaterialProperty<U, is_ad> &
  declarePropertyDerivative(const std::string & base, const std::vector<VariableName> & c);

  template <typename U, bool is_ad = false>
  GenericMaterialProperty<U, is_ad> & declarePropertyDerivative(const std::string & base,
                                                                const std::vector<SymbolName> & c);

  template <typename U, bool is_ad = false>
  GenericMaterialProperty<U, is_ad> & declarePropertyDerivative(const std::string & base,
                                                                const SymbolName & c1,
                                                                const SymbolName & c2 = "",
                                                                const SymbolName & c3 = "");
  ///@}

  ///@{
  /**
   * Methods for retrieving derivative material properties
   * @tparam U The material property type
   * @param base The name of the property to take the derivative of
   * @param c The variable(s) to take the derivatives with respect to
   */
  template <typename U, bool is_ad = false>
  const GenericMaterialProperty<U, is_ad> &
  getMaterialPropertyDerivative(const std::string & base, const std::vector<VariableName> & c);

  template <typename U, bool is_ad = false>
  const GenericMaterialProperty<U, is_ad> &
  getMaterialPropertyDerivative(const std::string & base, const std::vector<SymbolName> & c);

  template <typename U, bool is_ad = false>
  const GenericMaterialProperty<U, is_ad> &
  getMaterialPropertyDerivative(const std::string & base,
                                const SymbolName & c1,
                                const SymbolName & c2 = "",
                                const SymbolName & c3 = "");
  ///@}

  /**
   *@{ Convenience methods for retrieving derivative material properties based
   *   on a mix of variable names `c` and indices `v` into the
   *   _coupled_standard_moose_vars vector.
   */
  template <typename U, bool is_ad = false>
  const GenericMaterialProperty<U, is_ad> &
  getMaterialPropertyDerivative(const std::string & base,
                                const SymbolName & c1,
                                unsigned int v2,
                                unsigned int v3 = libMesh::invalid_uint);

  template <typename U, bool is_ad = false>
  const GenericMaterialProperty<U, is_ad> &
  getMaterialPropertyDerivative(const std::string & base,
                                unsigned int v1,
                                unsigned int v2 = libMesh::invalid_uint,
                                unsigned int v3 = libMesh::invalid_uint);
  ///@}

  ///@{
  /**
   * Methods for retrieving derivative material properties
   * @tparam U The material property type
   * @param base The name of the property to take the derivative of
   * @param c The variable(s) to take the derivatives with respect to
   */
  template <typename U, bool is_ad = false>
  const GenericMaterialProperty<U, is_ad> &
  getMaterialPropertyDerivativeByName(const MaterialPropertyName & base,
                                      const std::vector<VariableName> & c);

  template <typename U, bool is_ad = false>
  const GenericMaterialProperty<U, is_ad> &
  getMaterialPropertyDerivativeByName(const MaterialPropertyName & base,
                                      const std::vector<SymbolName> & c);

  template <typename U, bool is_ad = false>
  const GenericMaterialProperty<U, is_ad> &
  getMaterialPropertyDerivativeByName(const MaterialPropertyName & base,
                                      const SymbolName & c1,
                                      const SymbolName & c2 = "",
                                      const SymbolName & c3 = "");
  ///@}

  ///@{
  /**
   * check if derivatives of the passed in material property exist w.r.t a variable
   * that is _not_ coupled in to the current object
   */
  template <typename U, bool is_ad = false>
  void validateCoupling(const MaterialPropertyName & base,
                        const std::vector<VariableName> & c,
                        bool validate_aux = true);

  template <typename U, bool is_ad = false>
  void validateCoupling(const MaterialPropertyName & base,
                        const VariableName & c1 = "",
                        const VariableName & c2 = "",
                        const VariableName & c3 = "");

  template <typename U, bool is_ad = false>
  void validateNonlinearCoupling(const MaterialPropertyName & base,
                                 const VariableName & c1 = "",
                                 const VariableName & c2 = "",
                                 const VariableName & c3 = "");
  ///@}

  /**
   * Check if the material property base exists. Print a warning if it doesn't. This is
   * useful in materials that pull in only _derivative_ properties, which are optional.
   * If the base property name has a typo all derivatives will be set to zero without the
   * user ever knowing.
   */
  template <typename U, bool is_ad = false>
  void validateDerivativeMaterialPropertyBase(const std::string & base);

private:
  /// Check if a material property is present with the applicable restrictions
  template <typename U, bool is_ad = false>
  bool haveMaterialProperty(const std::string & prop_name);

  /// helper method to combine multiple VariableNames into a vector (if they are != "")
  std::vector<VariableName>
  buildVariableVector(const VariableName & c1, const VariableName & c2, const VariableName & c3);

  /// helper method to compile list of missing coupled variables for a given system
  template <typename U, bool is_ad = false>
  void validateCouplingHelper(const MaterialPropertyName & base,
                              const std::vector<VariableName> & c,
                              const System & system,
                              std::vector<VariableName> & missing);

  // check if the specified variable name is not the variable this kernel is acting on (always true
  // for any other type of object)
  bool isNotObjectVariable(const VariableName & name);

  /// Reference to FEProblemBase
  FEProblemBase & _dmi_fe_problem;
};

template <class T>
DerivativeMaterialInterface<T>::DerivativeMaterialInterface(const InputParameters & parameters)
  : T(parameters),
    _dmi_fe_problem(*parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"))
{
}

template <>
template <typename U, bool is_ad>
bool
DerivativeMaterialInterface<Material>::haveMaterialProperty(const std::string & prop_name)
{
  return ((this->boundaryRestricted() &&
           this->template hasBoundaryMaterialProperty<U, is_ad>(prop_name)) ||
          (this->template hasBlockMaterialProperty<U, is_ad>(prop_name)));
}

template <class T>
template <typename U, bool is_ad>
bool
DerivativeMaterialInterface<T>::haveMaterialProperty(const std::string & prop_name)
{
  // Call the correct method to test for material property declarations
  BlockRestrictable * blk = dynamic_cast<BlockRestrictable *>(this);
  BoundaryRestrictable * bnd = dynamic_cast<BoundaryRestrictable *>(this);
  return ((bnd && bnd->boundaryRestricted() &&
           bnd->template hasBoundaryMaterialProperty<U, is_ad>(prop_name)) ||
          (blk && blk->template hasBlockMaterialProperty<U, is_ad>(prop_name)) ||
          (this->template hasGenericMaterialProperty<U, is_ad>(prop_name)));
}

template <class T>
template <typename U, bool is_ad>
const GenericMaterialProperty<U, is_ad> &
DerivativeMaterialInterface<T>::getDefaultMaterialProperty(const std::string & name)
{
  // get the base property name
  std::string prop_name = this->deducePropertyName(name);

  // Check if it's just a constant
  const auto * default_property =
      this->template defaultGenericMaterialProperty<U, is_ad>(prop_name);
  if (default_property)
    return *default_property;

  // if found return the requested property
  return this->template getGenericZeroMaterialPropertyByName<U, is_ad>(prop_name);
}

template <class T>
template <typename U, bool is_ad>
const GenericMaterialProperty<U, is_ad> &
DerivativeMaterialInterface<T>::getDefaultMaterialPropertyByName(const std::string & prop_name)
{
  // TODO: deprecate this
  return this->template getGenericZeroMaterialPropertyByName<U, is_ad>(prop_name);
}

template <class T>
template <typename U, bool is_ad>
GenericMaterialProperty<U, is_ad> &
DerivativeMaterialInterface<T>::declarePropertyDerivative(const std::string & base,
                                                          const std::vector<VariableName> & c)
{
  std::vector<SymbolName> symbol_vector(c.begin(), c.end());
  return declarePropertyDerivative(base, symbol_vector);
}

template <class T>
template <typename U, bool is_ad>
GenericMaterialProperty<U, is_ad> &
DerivativeMaterialInterface<T>::declarePropertyDerivative(const std::string & base,
                                                          const std::vector<SymbolName> & c)
{
  return this->template declareGenericProperty<U, is_ad>(derivativePropertyName(base, c));
}

template <class T>
template <typename U, bool is_ad>
GenericMaterialProperty<U, is_ad> &
DerivativeMaterialInterface<T>::declarePropertyDerivative(const std::string & base,
                                                          const SymbolName & c1,
                                                          const SymbolName & c2,
                                                          const SymbolName & c3)
{
  if (c3 != "")
    return this->template declareGenericProperty<U, is_ad>(
        derivativePropertyNameThird(base, c1, c2, c3));
  if (c2 != "")
    return this->template declareGenericProperty<U, is_ad>(
        derivativePropertyNameSecond(base, c1, c2));
  return this->template declareGenericProperty<U, is_ad>(derivativePropertyNameFirst(base, c1));
}

template <class T>
template <typename U, bool is_ad>
const GenericMaterialProperty<U, is_ad> &
DerivativeMaterialInterface<T>::getMaterialPropertyDerivative(const std::string & base,
                                                              const std::vector<VariableName> & c)
{
  std::vector<SymbolName> symbol_vector(c.begin(), c.end());
  return getMaterialPropertyDerivative(base, symbol_vector);
}

template <class T>
template <typename U, bool is_ad>
const GenericMaterialProperty<U, is_ad> &
DerivativeMaterialInterface<T>::getMaterialPropertyDerivative(const std::string & base,
                                                              const std::vector<SymbolName> & c)
{
  // get the base property name
  std::string prop_name = this->deducePropertyName(base);

  /**
   * Check if base is a default property and shortcut to returning zero, as
   * derivatives of constants are zero.
   */
  if (this->template defaultGenericMaterialProperty<U, is_ad>(prop_name))
    return this->template getGenericZeroMaterialProperty<U, is_ad>();

  return this->template getGenericZeroMaterialPropertyByName<U, is_ad>(
      derivativePropertyName(prop_name, c));
}

template <class T>
template <typename U, bool is_ad>
const GenericMaterialProperty<U, is_ad> &
DerivativeMaterialInterface<T>::getMaterialPropertyDerivative(const std::string & base,
                                                              const SymbolName & c1,
                                                              const SymbolName & c2,
                                                              const SymbolName & c3)
{
  // get the base property name
  std::string prop_name = this->deducePropertyName(base);

  /**
   * Check if base is a default property and shortcut to returning zero, as
   * derivatives of constants are zero.
   */
  if (this->template defaultGenericMaterialProperty<U, is_ad>(prop_name))
    return this->template getGenericZeroMaterialProperty<U, is_ad>();

  if (c3 != "")
    return this->template getGenericZeroMaterialPropertyByName<U, is_ad>(
        derivativePropertyNameThird(prop_name, c1, c2, c3));
  if (c2 != "")
    return this->template getGenericZeroMaterialPropertyByName<U, is_ad>(
        derivativePropertyNameSecond(prop_name, c1, c2));
  return this->template getGenericZeroMaterialPropertyByName<U, is_ad>(
      derivativePropertyNameFirst(prop_name, c1));
}

template <class T>
template <typename U, bool is_ad>
const GenericMaterialProperty<U, is_ad> &
DerivativeMaterialInterface<T>::getMaterialPropertyDerivative(const std::string & base,
                                                              const SymbolName & c1,
                                                              unsigned int v2,
                                                              unsigned int v3)
{
  return getMaterialPropertyDerivative<U, is_ad>(
      base,
      c1,
      this->_coupled_standard_moose_vars[v2]->name(),
      v3 == libMesh::invalid_uint ? "" : this->_coupled_standard_moose_vars[v3]->name());
}

template <class T>
template <typename U, bool is_ad>
const GenericMaterialProperty<U, is_ad> &
DerivativeMaterialInterface<T>::getMaterialPropertyDerivative(const std::string & base,
                                                              unsigned int v1,
                                                              unsigned int v2,
                                                              unsigned int v3)
{
  return getMaterialPropertyDerivative<U, is_ad>(
      base,
      this->_coupled_standard_moose_vars[v1]->name(),
      v2 == libMesh::invalid_uint ? "" : this->_coupled_standard_moose_vars[v2]->name(),
      v3 == libMesh::invalid_uint ? "" : this->_coupled_standard_moose_vars[v3]->name());
}

template <class T>
template <typename U, bool is_ad>
const GenericMaterialProperty<U, is_ad> &
DerivativeMaterialInterface<T>::getMaterialPropertyDerivativeByName(
    const MaterialPropertyName & base, const std::vector<VariableName> & c)
{
  std::vector<SymbolName> symbol_vector(c.begin(), c.end());
  return getMaterialPropertyDerivativeByName(base, symbol_vector);
}

template <class T>
template <typename U, bool is_ad>
const GenericMaterialProperty<U, is_ad> &
DerivativeMaterialInterface<T>::getMaterialPropertyDerivativeByName(
    const MaterialPropertyName & base, const std::vector<SymbolName> & c)
{
  return this->template getGenericZeroMaterialPropertyByName<U, is_ad>(
      derivativePropertyName(base, c));
}

template <class T>
template <typename U, bool is_ad>
const GenericMaterialProperty<U, is_ad> &
DerivativeMaterialInterface<T>::getMaterialPropertyDerivativeByName(
    const MaterialPropertyName & base,
    const SymbolName & c1,
    const SymbolName & c2,
    const SymbolName & c3)
{
  if (c3 != "")
    return this->template getGenericZeroMaterialPropertyByName<U, is_ad>(
        derivativePropertyNameThird(base, c1, c2, c3));
  if (c2 != "")
    return this->template getGenericZeroMaterialPropertyByName<U, is_ad>(
        derivativePropertyNameSecond(base, c1, c2));
  return this->template getGenericZeroMaterialPropertyByName<U, is_ad>(
      derivativePropertyNameFirst(base, c1));
}

template <class T>
template <typename U, bool is_ad>
void
DerivativeMaterialInterface<T>::validateCouplingHelper(const MaterialPropertyName & base,
                                                       const std::vector<VariableName> & c,
                                                       const System & system,
                                                       std::vector<VariableName> & missing)
{
  unsigned int ncoupled = this->_coupled_standard_moose_vars.size();

  // iterate over all variables in the current system (in groups)
  for (unsigned int i = 0; i < system.n_variable_groups(); ++i)
  {
    const VariableGroup & vg = system.variable_group(i);
    for (unsigned int j = 0; j < vg.n_variables(); ++j)
    {
      std::vector<SymbolName> cj(c.begin(), c.end());
      SymbolName jname = vg.name(j);
      cj.push_back(jname);

      // if the derivative exists make sure the variable is coupled
      if (haveMaterialProperty<U, is_ad>(derivativePropertyName(base, cj)))
      {
        // kernels and BCs to not have the variable they are acting on in coupled_moose_vars
        bool is_missing = isNotObjectVariable(jname);

        for (unsigned int k = 0; k < ncoupled; ++k)
          if (this->_coupled_standard_moose_vars[k]->name() == jname)
          {
            is_missing = false;
            break;
          }

        if (is_missing)
          missing.push_back(jname);
      }
    }
  }
}

template <class T>
template <typename U, bool is_ad>
void
DerivativeMaterialInterface<T>::validateCoupling(const MaterialPropertyName & base,
                                                 const std::vector<VariableName> & c,
                                                 bool validate_aux)
{
  // get the base property name
  std::string prop_name = this->deducePropertyName(base);
  // list of potentially missing coupled variables
  std::vector<VariableName> missing;

  // iterate over all variables in the both the non-linear and auxiliary system (optional)
  validateCouplingHelper<U, is_ad>(
      prop_name, c, _dmi_fe_problem.getNonlinearSystemBase().system(), missing);
  if (validate_aux)
    validateCouplingHelper<U, is_ad>(
        prop_name, c, _dmi_fe_problem.getAuxiliarySystem().system(), missing);

  if (missing.size() > 0)
  {
    // join list of missing variable names
    std::string list = missing[0];
    for (unsigned int i = 1; i < missing.size(); ++i)
      list += ", " + missing[i];

    mooseWarning("Missing coupled variables {",
                 list,
                 "} (add them to coupled_variables parameter of ",
                 this->name(),
                 ")");
  }
}

template <class T>
std::vector<VariableName>
DerivativeMaterialInterface<T>::buildVariableVector(const VariableName & c1,
                                                    const VariableName & c2,
                                                    const VariableName & c3)
{
  std::vector<VariableName> c;
  if (c1 != "")
  {
    c.push_back(c1);
    if (c2 != "")
    {
      c.push_back(c2);
      if (c3 != "")
        c.push_back(c3);
    }
  }
  return c;
}

template <class T>
template <typename U, bool is_ad>
void
DerivativeMaterialInterface<T>::validateCoupling(const MaterialPropertyName & base,
                                                 const VariableName & c1,
                                                 const VariableName & c2,
                                                 const VariableName & c3)
{
  validateCoupling<U, is_ad>(base, buildVariableVector(c1, c2, c3), true);
}

template <class T>
template <typename U, bool is_ad>
void
DerivativeMaterialInterface<T>::validateNonlinearCoupling(const MaterialPropertyName & base,
                                                          const VariableName & c1,
                                                          const VariableName & c2,
                                                          const VariableName & c3)
{
  validateCoupling<U, is_ad>(base, buildVariableVector(c1, c2, c3), false);
}

template <class T>
template <typename U, bool is_ad>
void
DerivativeMaterialInterface<T>::validateDerivativeMaterialPropertyBase(const std::string & base)
{
  // resolve the input parameter name base to the actual material property name
  const MaterialPropertyName prop_name = this->template getParam<MaterialPropertyName>(base);

  // check if the material property does not exist on the blocks of the current object,
  // and check if it is not a plain number in the input file
  if (!haveMaterialProperty<U, is_ad>(prop_name) &&
      this->template defaultGenericMaterialProperty<U, is_ad>(prop_name) == 0)
    mooseWarning("The material property '",
                 prop_name,
                 "' does not exist. The kernel '",
                 this->name(),
                 "' only needs its derivatives, but this may indicate a typo in the input file.");
}

template <class T>
inline bool
DerivativeMaterialInterface<T>::isNotObjectVariable(const VariableName & name)
{
  // try to cast this to a Kernel pointer
  KernelBase * kernel_ptr = dynamic_cast<KernelBase *>(this);
  if (kernel_ptr != nullptr)
    return kernel_ptr->variable().name() != name;

  // try to cast this to a BoundaryCondition pointer
  BoundaryCondition * bc_ptr = dynamic_cast<BoundaryCondition *>(this);
  if (bc_ptr != nullptr)
    return bc_ptr->variable().name() != name;

  // This interface is not templated on a class derived from either Kernel or BC
  return true;
}
