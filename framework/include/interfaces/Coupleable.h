//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <unordered_map>
#include "MooseTypes.h"
#include "MooseArray.h"
#include "MooseVariableFE.h"
#include "MooseVariableFV.h"
#include "MooseLinearVariableFV.h"
#include "InputParameters.h"
#include "HasMembers.h"

#define usingCoupleableMembers                                                                     \
  using Coupleable::_zero;                                                                         \
  using Coupleable::_grad_zero;                                                                    \
  using Coupleable::_ad_zero;                                                                      \
  using Coupleable::_ad_grad_zero

// Forward declarations
class MooseVariableScalar;
class MooseObject;

namespace libMesh
{
template <typename T>
class DenseVector;
}

template <typename>
class MooseVariableField;
typedef MooseVariableField<Real> MooseWritableVariable;

/**
 * Interface for objects that needs coupling capabilities
 *
 */
class Coupleable
{
public:
  /**
   * Constructing the object
   * @param parameters Parameters that come from constructing the object
   * @param nodal true if we need to couple with nodal values, otherwise false
   * @param is_fv Whether the \p MooseObject is a finite volume object
   */
  Coupleable(const MooseObject * moose_object, bool nodal, bool is_fv = false);

  /**
   * Get the list of coupled variables
   * @return The list of coupled variables
   */
  const std::unordered_map<std::string, std::vector<MooseVariableFieldBase *>> &
  getCoupledVars() const
  {
    return _coupled_vars;
  }

  /**
   * Get the list of all coupled variables
   * @return The list of all coupled variables
   */
  const std::vector<MooseVariableFieldBase *> & getCoupledMooseVars() const
  {
    return _coupled_moose_vars;
  }

  /**
   * Get the list of standard coupled variables
   * @return The list of standard coupled variables
   */
  const std::vector<MooseVariable *> & getCoupledStandardMooseVars() const
  {
    return _coupled_standard_moose_vars;
  }

  /**
   * Get the list of vector coupled variables
   * @return The list of vector coupled variables
   */
  const std::vector<VectorMooseVariable *> & getCoupledVectorMooseVars() const
  {
    return _coupled_vector_moose_vars;
  }

  /**
   * Get the list of array coupled variables
   * @return The list of array coupled variables
   */
  const std::vector<ArrayMooseVariable *> & getCoupledArrayMooseVars() const
  {
    return _coupled_array_moose_vars;
  }

  void addFEVariableCoupleableVectorTag(TagID tag) { _fe_coupleable_vector_tags.insert(tag); }

  void addFEVariableCoupleableMatrixTag(TagID tag) { _fe_coupleable_matrix_tags.insert(tag); }

  std::set<TagID> & getFEVariableCoupleableVectorTags() { return _fe_coupleable_vector_tags; }

  std::set<TagID> & getFEVariableCoupleableMatrixTags() { return _fe_coupleable_matrix_tags; }

  const std::set<TagID> & getFEVariableCoupleableVectorTags() const
  {
    return _fe_coupleable_vector_tags;
  }

  const std::set<TagID> & getFEVariableCoupleableMatrixTags() const
  {
    return _fe_coupleable_matrix_tags;
  }

  /**
   * returns a reference to the set of writable coupled variables
   */
  auto & getWritableCoupledVariables() const { return _writable_coupled_variables[_c_tid]; }

  /**
   * Checks whether the object has any writable coupled variables
   */
  bool hasWritableCoupledVariables() const { return !getWritableCoupledVariables().empty(); }

protected:
  /**
   * A call-back function provided by the derived object for actions before coupling a variable
   * with functions such as coupledValue.
   */
  virtual void coupledCallback(const std::string & /*var_name*/, bool /*is_old*/) const {}

  /**
   * Returns true if a variables has been coupled as name.
   * @param var_name The name the kernel wants to refer to the variable as.
   * @param i By default 0, in general the index to test in a vector of MooseVariable pointers.
   * @return True if a coupled variable has the supplied name
   */
  virtual bool isCoupled(const std::string & var_name, unsigned int i = 0) const;

  /**
   * Returns true if a variable passed as a coupled value is really a constant
   * @param var_name The name the kernel wants to refer to the variable as.
   * @return True if the variable is actually a constant
   */
  virtual bool isCoupledConstant(const std::string & var_name) const;

  /**
   * Number of coupled components
   * @param var_name Name of the variable
   * @return number of components this variable has (usually 1)
   */
  unsigned int coupledComponents(const std::string & var_name) const;

  /**
   * Names of the variable in the Coupleable interface
   * @param var_name Name of the variable
   * @param comp the component of the variable
   * @return name the variable has been coupled as. For constants, returns the constant
   */
  VariableName coupledName(const std::string & var_name, unsigned int comp = 0) const;

  /**
   * Names of the variables in the Coupleable interface
   * @param var_name Names of the variables
   * @return names the variables have been coupled as
   */
  std::vector<VariableName> coupledNames(const std::string & var_name) const;

  /**
   * Returns the index for a coupled variable by name
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Index of coupled variable, if this is an optionally coupled variable that wasn't
   * provided this will return a unique "invalid" index.
   */
  virtual unsigned int coupled(const std::string & var_name, unsigned int comp = 0) const;

  /**
   * Returns the indices for a coupled variable's components
   * @param var_name Name of coupled variable
   * @return Vector of the indices for all components of the coupled variable \p var_name.
   */
  std::vector<unsigned int> coupledIndices(const std::string & var_name) const;

  /**
   * Returns value of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the coupled variable
   * @see Kernel::_u
   */
  virtual const VariableValue & coupledValue(const std::string & var_name,
                                             unsigned int comp = 0) const;

  /**
   * Returns the values for all of a coupled variable components
   * @param var_name Name of coupled variable
   * @return Vector of VariableValue pointers for each component of \p var_name
   */
  std::vector<const VariableValue *> coupledValues(const std::string & var_name) const;

  /**
   * Returns the values for all of a coupled vector variable's components
   * @param var_name Name of coupled variable
   * @return Vector of VectorVariableValue pointers for each component of \p var_name
   */
  std::vector<const VectorVariableValue *> coupledVectorValues(const std::string & var_name) const;

  /**
   * Returns value of a coupled variable for use in templated automatic differentiation classes
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a GenericVariableValue for the coupled variable
   */
  template <bool is_ad>
  const GenericVariableValue<is_ad> & coupledGenericValue(const std::string & var_name,
                                                          unsigned int comp = 0) const;

  /**
   * Returns the values for all of a coupled variable's components for use in templated automatic
   * differentiation classes
   * @param var_name Name of coupled variable
   * @return Vector of GenericVariableValue pointers for each component of \p var_name
   */
  template <bool is_ad>
  std::vector<const GenericVariableValue<is_ad> *>
  coupledGenericValues(const std::string & var_name) const;

  /**
   * Returns DOF value of a coupled variable for use in templated automatic differentiation classes
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a GenericVariableValue for the coupled variable
   */
  template <bool is_ad>
  const GenericVariableValue<is_ad> & coupledGenericDofValue(const std::string & var_name,
                                                             unsigned int comp = 0) const;

  /**
   * Returns time derivative of a coupled variable for use in templated automatic differentiation
   * classes
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a GenericVariableValue for the coupled variable time derivative
   */
  template <bool is_ad>
  const GenericVariableValue<is_ad> & coupledGenericDot(const std::string & var_name,
                                                        unsigned int comp = 0) const;

  /**
   * Returns the second time derivative of a coupled variable for use in templated automatic
   * differentiation classes
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a GenericVariableValue for the coupled variable second time derivative
   */
  template <bool is_ad>
  const GenericVariableValue<is_ad> & coupledGenericDotDot(const std::string & var_name,
                                                           unsigned int comp = 0) const;

  /**
   * Returns value of a coupled lower-dimensional variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the coupled variable
   */
  virtual const VariableValue & coupledValueLower(const std::string & var_name,
                                                  unsigned int comp = 0) const;

  /**
   * Returns value of a coupled variable for use in Automatic Differentiation
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a ADVariableValue for the coupled variable
   */
  const ADVariableValue & adCoupledValue(const std::string & var_name, unsigned int comp = 0) const;

  /**
   * Returns the values for all of a coupled variable's components for use in Automatic
   * Differentiation
   * @param var_name Name of coupled variable
   * @return Vector of ADVariableValue pointers for each component of \p var_name
   */
  std::vector<const ADVariableValue *> adCoupledValues(const std::string & var_name) const;

  /**
   * Returns value of a coupled lower-dimensional variable for use in Automatic Differentiation
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a ADVariableValue for the coupled variable
   */
  const ADVariableValue & adCoupledLowerValue(const std::string & var_name,
                                              unsigned int comp = 0) const;

  /**
   * Returns value of a coupled vector variable for use in Automatic Differentiation
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the coupled variable
   * @see Kernel::value
   */
  const ADVectorVariableValue & adCoupledVectorValue(const std::string & var_name,
                                                     unsigned int comp = 0) const;

  /**
   * Returns the values for all of a coupled vector variable's components for use in
   * Automatic Differentiation
   * @param var_name Name of coupled variable
   * @return Vector of ADVariableValue pointers for each component of \p var_name
   */
  std::vector<const ADVectorVariableValue *>
  adCoupledVectorValues(const std::string & var_name) const;

  /**
   * Returns value of a coupled variable for a given tag
   * @param var_names Name(s) of coupled variable(s)
   * @param tag vector tag ID
   * @param index Index of the desired variable in the vector of coupled variables
   * @return Reference to a VariableValue for the coupled variable
   * @see Kernel::_u
   */
  virtual const VariableValue &
  coupledVectorTagValue(const std::string & var_names, TagID tag, unsigned int index = 0) const;

  virtual const VariableValue & coupledVectorTagValue(const std::string & var_names,
                                                      const std::string & tag_name,
                                                      unsigned int index = 0) const;

  /**
   * Returns the values for all the coupled variables desired for a given tag
   * @param var_names Name(s) of coupled variable(s)
   * @param tag vector tag ID
   * @return Vector of VariableValue pointers for each variable in \p var_names
   */
  std::vector<const VariableValue *> coupledVectorTagValues(const std::string & var_names,
                                                            TagID tag) const;

  std::vector<const VariableValue *> coupledVectorTagValues(const std::string & var_names,
                                                            const std::string & tag_name) const;

  /**
   * Returns value of a coupled array variable for a given tag
   * @param var_names Name(s) of coupled array variable(s)
   * @param tag vector tag ID
   * @param index Index of the desired variable in the vector of coupled variables
   * @return Reference to a VariableValue for the coupled array variable
   * @see Kernel::_u
   */
  virtual const ArrayVariableValue & coupledVectorTagArrayValue(const std::string & var_names,
                                                                TagID tag,
                                                                unsigned int index = 0) const;

  virtual const ArrayVariableValue & coupledVectorTagArrayValue(const std::string & var_names,
                                                                const std::string & tag_name,
                                                                unsigned int index = 0) const;

  /**
   * Returns the values for all the coupled variables desired for a given tag
   * @param var_name Name of array coupled variable
   * @param tag vector tag ID
   * @return Vector of ArrayVariableValue pointers for each variable in \p var_names
   */
  std::vector<const ArrayVariableValue *> coupledVectorTagArrayValues(const std::string & var_names,
                                                                      TagID tag) const;

  std::vector<const ArrayVariableValue *>
  coupledVectorTagArrayValues(const std::string & var_names, const std::string & tag_name) const;

  /**
   * Returns gradient of a coupled variable for a given tag
   * @param var_names Name(s) of coupled variable(s)
   * @param tag vector tag ID
   * @param index Index of the desired variable in the vector of coupled variables
   * @return Reference to a VariableGradient containing the gradient of the coupled variable
   * @see Kernel::gradient
   */
  virtual const VariableGradient &
  coupledVectorTagGradient(const std::string & var_names, TagID tag, unsigned int index = 0) const;

  virtual const VariableGradient & coupledVectorTagGradient(const std::string & var_names,
                                                            const std::string & tag_name,
                                                            unsigned int index = 0) const;

  /**
   * Returns gradients for all the coupled variables desired for a given tag
   * @param var_names Name(s) of coupled array variable(s)
   * @param tag vector tag ID
   * @return Vector of VariableGradient pointers for each variables in \p var_name
   */
  std::vector<const VariableGradient *> coupledVectorTagGradients(const std::string & var_names,
                                                                  TagID tag) const;

  std::vector<const VariableGradient *>
  coupledVectorTagGradients(const std::string & var_names, const std::string & tag_name) const;

  /**
   * Returns gradient of a coupled array variable for a given tag
   * @param var_names Name(s) of coupled array variable(s)
   * @param tag vector tag ID
   * @param index Index of the desired variable in the vector of coupled variables
   * @return Reference to a ArrayVariableGradient containing the gradient of the coupled array
   * variable
   * @see Kernel::gradient
   */
  virtual const ArrayVariableGradient & coupledVectorTagArrayGradient(const std::string & var_names,
                                                                      TagID tag,
                                                                      unsigned int index = 0) const;

  virtual const ArrayVariableGradient & coupledVectorTagArrayGradient(const std::string & var_names,
                                                                      const std::string & tag_name,
                                                                      unsigned int index = 0) const;

  /**
   * Returns gradients for all the coupled variables desired for a given tag
   * @param var_names Name(s) of coupled array variable(s)
   * @param tag vector tag ID
   * @return Vector of ArrayVariableGradient pointers for each variable in \p var_name
   */
  std::vector<const ArrayVariableGradient *>
  coupledVectorTagArrayGradients(const std::string & var_names, TagID tag) const;

  std::vector<const ArrayVariableGradient *>
  coupledVectorTagArrayGradients(const std::string & var_names, const std::string & tag_name) const;

  /**
   * Returns dof value of a coupled variable for a given tag
   * @param var_names Name(s) of coupled variable(s)
   * @param tag vector tag ID
   * @param index Index of the desired variable in the vector of coupled variables
   * @return Reference to a DofValue for the coupled variable
   */
  virtual const VariableValue &
  coupledVectorTagDofValue(const std::string & var_name, TagID tag, unsigned int index = 0) const;

  virtual const VariableValue & coupledVectorTagDofValue(const std::string & var_names,
                                                         const std::string & tag_name,
                                                         unsigned int index = 0) const;

  /**
   * Returns evaluations of a tagged vector at the requested variable's degree of freedom indices
   * @param var_name Name of coupled variable
   * @param tag_name vector tag name
   * @return Reference to a ArrayVariableValue for the coupled variable
   */
  const ArrayVariableValue & coupledVectorTagArrayDofValue(const std::string & var_name,
                                                           const std::string & tag_name,
                                                           unsigned int comp = 0) const;

  /**
   * Returns the dof values for all the coupled variables desired for a given tag
   * @param var_names Name(s) of coupled variable(s)
   * @param tag vector tag ID
   * @return Vector of VariableValue pointers for each variable in \p var_name
   */
  std::vector<const VariableValue *> coupledVectorTagDofValues(const std::string & var_names,
                                                               TagID tag) const;

  std::vector<const VariableValue *> coupledVectorTagDofValues(const std::string & var_names,
                                                               const std::string & tag_name) const;

  /**
   * Returns value of a coupled variable for a given tag. This couples the diag vector of matrix
   * @param var_names Name(s) of coupled variable(s)
   * @param tag matrix tag ID
   * @param index Index of the desired variable in the vector of coupled variables
   * @return Reference to a VariableValue for the coupled variable
   * @see Kernel::_u
   */
  virtual const VariableValue &
  coupledMatrixTagValue(const std::string & var_names, TagID tag, unsigned int index = 0) const;

  virtual const VariableValue & coupledMatrixTagValue(const std::string & var_names,
                                                      const std::string & tag_name,
                                                      unsigned int index = 0) const;

  /**
   * Returns the diagonal matrix values for all the coupled variables desired for a given tag
   * @param var_names Name(s) of coupled variable(s)
   * @param tag matrix tag ID
   * @return Vector of VariableValue pointers for each variable in \p var_name
   */
  std::vector<const VariableValue *> coupledMatrixTagValues(const std::string & var_names,
                                                            TagID tag) const;

  std::vector<const VariableValue *> coupledMatrixTagValues(const std::string & var_names,
                                                            const std::string & tag_name) const;

  /**
   * Returns value of a coupled vector variable
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VectorVariableValue for the coupled vector variable
   * @see VectorKernel::_u
   */
  virtual const VectorVariableValue & coupledVectorValue(const std::string & var_name,
                                                         unsigned int comp = 0) const;

  /**
   * Returns value of a coupled array variable
   * @param var_name Name of coupled array variable
   * @param comp Component number for vector of coupled array variables
   * @return Reference to a ArrayVariableValue for the coupled vector variable
   * @see ArrayKernel::_u
   */
  virtual const ArrayVariableValue & coupledArrayValue(const std::string & var_name,
                                                       unsigned int comp = 0) const;

  /**
   * Returns the values for all of a coupled array variable's components
   * @param var_name Name of coupled array variable
   * @return Vector of ArrayVariableValue pointers for each component of \p var_name
   */
  std::vector<const ArrayVariableValue *> coupledArrayValues(const std::string & var_name) const;

  /**
   * Returns a *writable* MooseVariable object for a nodal or elemental variable. Use
   * var.setNodalValue(val[, idx]) in both cases (!) to set the solution DOF values. Only one
   * object can obtain a writable reference in a simulation. Note that the written values will
   * not ba available in the same system loop! E.g. values written using this API by a nodal
   * AuxKernel will not be updated for other nodal AuxKernels during the same iteration over all
   * nodes.
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a MooseWritableVariable for the coupled variable
   * @see Kernel::value
   */
  MooseWritableVariable & writableVariable(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns a *writable* reference to a coupled variable for writing to multiple
   * AuxVariables from a single AuxKernel or a UserObject. Only one object can obtain
   * a writable reference in a simulation.
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the coupled variable
   * @see Kernel::value
   */
  virtual VariableValue & writableCoupledValue(const std::string & var_name, unsigned int comp = 0);

  /**
   * Checks that the passed in variable is only accessed writable by one object in a given subdomain
   */
  void checkWritableVar(MooseWritableVariable * var);

  /**
   * Returns an old value from previous time step  of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the old value of the coupled variable
   * @see Kernel::valueOld
   */
  virtual const VariableValue & coupledValueOld(const std::string & var_name,
                                                unsigned int comp = 0) const;

  /**
   * Returns the old values for all of a coupled variable's components
   * @param var_name Name of coupled variable
   * @return Vector of VariableValue pointers for each component of \p var_name
   */
  std::vector<const VariableValue *> coupledValuesOld(const std::string & var_name) const;

  /**
   * Returns an old value from two time steps previous of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the older value of the coupled variable
   * @see Kernel::valueOlder
   */
  virtual const VariableValue & coupledValueOlder(const std::string & var_name,
                                                  unsigned int comp = 0) const;

  /**
   * Returns the older values for all of a coupled variable's components
   * @param var_name Name of coupled variable
   * @return Vector of VariableValue pointers for each component of \p var_name
   */
  std::vector<const VariableValue *> coupledValuesOlder(const std::string & var_name) const;

  /**
   * Returns value of previous Newton iterate of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the older value of the coupled variable
   */
  virtual const VariableValue & coupledValuePreviousNL(const std::string & var_name,
                                                       unsigned int comp = 0) const;

  /**
   * Returns an old value from previous time step  of a coupled vector variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VectorVariableValue containing the old value of the coupled variable
   * @see VectorKernel::_u_old
   */
  virtual const VectorVariableValue & coupledVectorValueOld(const std::string & var_name,
                                                            unsigned int comp = 0) const;

  /**
   * Returns an old value from two time steps previous of a coupled vector variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VectorVariableValue containing the older value of the coupled variable
   * @see VectorKernel::_u_older
   */
  virtual const VectorVariableValue & coupledVectorValueOlder(const std::string & var_name,
                                                              unsigned int comp = 0) const;

  /**
   * Returns an old value from previous time step of a coupled array variable
   * @param var_name Name of coupled array variable
   * @param comp Component number for vector of coupled array variables
   * @return Reference to a ArrayVariableValue containing the old value of the coupled variable
   * @see ArrayKernel::_u_old
   */
  virtual const ArrayVariableValue & coupledArrayValueOld(const std::string & var_name,
                                                          unsigned int comp = 0) const;

  /**
   * Returns an old value from two time steps previous of a coupled array variable
   * @param var_name Name of coupled array variable
   * @param comp Component number for vector of coupled array variables
   * @return Reference to a ArrayVariableValue containing the older value of the coupled variable
   * @see ArrayKernel::_u_older
   */
  virtual const ArrayVariableValue & coupledArrayValueOlder(const std::string & var_name,
                                                            unsigned int comp = 0) const;

  /**
   * Returns gradient of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableGradient containing the gradient of the coupled variable
   * @see Kernel::gradient
   */
  virtual const VariableGradient & coupledGradient(const std::string & var_name,
                                                   unsigned int comp = 0) const;

  /**
   * Returns the gradients for all of a coupled variable's components
   * @param var_name Name of coupled variable
   * @return Vector of VariableGradient pointers for each component of \p var_name
   */
  std::vector<const VariableGradient *> coupledGradients(const std::string & var_name) const;

  /**
   * Returns gradient of a coupled variable for use in Automatic Differentiation
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to an ADVariableGradient containing the gradient of the coupled variable
   * @see Kernel::gradient
   */
  const ADVariableGradient & adCoupledGradient(const std::string & var_name,
                                               unsigned int comp = 0) const;

  /**
   * Returns gradient of a coupled variable's time derivative for use in Automatic Differentiation
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to an ADVariableGradient containing the gradient of the coupled variable's
   * time derivative
   */
  const ADVariableGradient & adCoupledGradientDot(const std::string & var_name,
                                                  unsigned int comp = 0) const;

  /**
   * Returns the gradients for all of a coupled variable's components for use in Automatic
   * Differentiation
   * @param var_name Name of coupled variable
   * @return Vector of ADVariableGradient pointers for each component of \p var_name
   */
  std::vector<const ADVariableGradient *> adCoupledGradients(const std::string & var_name) const;

  /**
   * Returns gradient of a coupled variable for use in templated automatic differentiation
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableGradient containing the gradient of the coupled variable
   * @see Kernel::gradient
   */
  template <bool is_ad>
  const GenericVariableGradient<is_ad> & coupledGenericGradient(const std::string & var_name,
                                                                unsigned int comp = 0) const;

  /**
   * Returns the gradients for all of a coupled variable's components for use in templated automatic
   * differentiation
   * @param var_name Name of coupled variable
   * @return Vector of VariableGradient pointers for each component of \p var_name
   */
  template <bool is_ad>
  std::vector<const GenericVariableGradient<is_ad> *>
  coupledGenericGradients(const std::string & var_name) const;

  /**
   * Returns gradient of a coupled vector variable for use in Automatic Differentiation
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VectorVariableGradient containing the gradient of the coupled variable
   * @see Kernel::gradient
   */
  const ADVectorVariableGradient & adCoupledVectorGradient(const std::string & var_name,
                                                           unsigned int comp = 0) const;

  /**
   * Returns second derivatives of a coupled variable for use in Automatic Differentiation
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableSecond containing the second derivatives of the coupled variable
   */
  const ADVariableSecond & adCoupledSecond(const std::string & var_name,
                                           unsigned int comp = 0) const;

  /**
   * Returns second derivatives of a coupled vector variable for use in Automatic Differentiation
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VectorVariableSecond containing the second derivatives of the coupled
   * variable
   */
  const ADVectorVariableSecond & adCoupledVectorSecond(const std::string & var_name,
                                                       unsigned int comp = 0) const;

  /**
   * Returns an old gradient from previous time step of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableGradient containing the old gradient of the coupled variable
   * @see Kernel::gradientOld
   */
  virtual const VariableGradient & coupledGradientOld(const std::string & var_name,
                                                      unsigned int comp = 0) const;

  /**
   * Returns the old gradients for all of a coupled variable's components
   * @param var_name Name of coupled variable
   * @return Vector of VariableGradient pointers for each component of \p var_name
   */
  std::vector<const VariableGradient *> coupledGradientsOld(const std::string & var_name) const;

  /**
   * Returns an old gradient from two time steps previous of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableGradient containing the older gradient of the coupled variable
   * @see Kernel::gradientOlder
   */
  virtual const VariableGradient & coupledGradientOlder(const std::string & var_name,
                                                        unsigned int comp = 0) const;

  /**
   * Returns gradient of a coupled variable for previous Newton iterate
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableGradient containing the gradient of the coupled variable
   */
  virtual const VariableGradient & coupledGradientPreviousNL(const std::string & var_name,
                                                             unsigned int comp = 0) const;

  /**
   * Time derivative of the gradient of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableGradient containing the time derivative of the gradient of a
   * coupled variable
   */
  virtual const VariableGradient & coupledGradientDot(const std::string & var_name,
                                                      unsigned int comp = 0) const;

  /**
   * Second time derivative of the gradient of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableGradient containing the time derivative of the gradient of a
   * coupled variable
   */
  virtual const VariableGradient & coupledGradientDotDot(const std::string & var_name,
                                                         unsigned int comp = 0) const;

  /**
   * Returns gradient of a coupled vector variable
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VectorVariableGradient containing the gradient of the coupled vector
   * variable
   */
  virtual const VectorVariableGradient & coupledVectorGradient(const std::string & var_name,
                                                               unsigned int comp = 0) const;

  /**
   * Returns an old gradient from previous time step of a coupled vector variable
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VectorVariableGradient containing the old gradient of the coupled vector
   * variable
   */
  virtual const VectorVariableGradient & coupledVectorGradientOld(const std::string & var_name,
                                                                  unsigned int comp = 0) const;

  /**
   * Returns an old gradient from two time steps previous of a coupled vector variable
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VectorVariableGradient containing the older gradient of the coupled
   * vector variable
   */
  virtual const VectorVariableGradient & coupledVectorGradientOlder(const std::string & var_name,
                                                                    unsigned int comp = 0) const;

  /**
   * Returns gradient of a coupled array variable
   * @param var_name Name of coupled array variable
   * @param comp Component number for vector of coupled array variables
   * @return Reference to a VectorVariableGradient containing the gradient of the coupled array
   * variable
   */
  virtual const ArrayVariableGradient & coupledArrayGradient(const std::string & var_name,
                                                             unsigned int comp = 0) const;

  /**
   * Returns an old gradient from previous time step of a coupled array variable
   * @param var_name Name of coupled array variable
   * @param comp Component number for vector of coupled array variables
   * @return Reference to a VectorVariableGradient containing the old gradient of the coupled array
   * variable
   */
  virtual const ArrayVariableGradient & coupledArrayGradientOld(const std::string & var_name,
                                                                unsigned int comp = 0) const;

  /**
   * Returns an old gradient from two time steps previous of a coupled array variable
   * @param var_name Name of coupled array variable
   * @param comp Component number for vector of coupled array variables
   * @return Reference to a ArrayVariableGradient containing the older gradient of the coupled
   * array variable
   */
  virtual const ArrayVariableGradient & coupledArrayGradientOlder(const std::string & var_name,
                                                                  unsigned int comp = 0) const;

  /**
   * Retun a gradient of a coupled array variable's time derivative
   * @param var_name Name of coupled array variable
   * @param comp Component number for vector of coupled array variables
   * @return Reference to a ArrayVariableGradient containing the gradient of the time derivative
   * the coupled array variable
   */
  virtual const ArrayVariableGradient & coupledArrayGradientDot(const std::string & var_name,
                                                                unsigned int comp = 0) const;

  /**
   * Returns curl of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VectorVariableCurl containing the curl of the coupled variable
   * @see Kernel::_curl_u
   */
  virtual const VectorVariableCurl & coupledCurl(const std::string & var_name,
                                                 unsigned int comp = 0) const;

  /**
   * Returns an old curl from previous time step of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VectorVariableCurl containing the old curl of the coupled variable
   * @see Kernel::_curl_u_old
   */
  virtual const VectorVariableCurl & coupledCurlOld(const std::string & var_name,
                                                    unsigned int comp = 0) const;

  /**
   * Returns an old curl from two time steps previous of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VectorVariableCurl containing the older curl of the coupled variable
   * @see Kernel::_curl_u_older
   */
  virtual const VectorVariableCurl & coupledCurlOlder(const std::string & var_name,
                                                      unsigned int comp = 0) const;

  /**
   * Returns divergence of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VectorVariableDivergence containing the divergence of the coupled
   * variable
   * @see Kernel::_div_u
   */
  virtual const VectorVariableDivergence & coupledDiv(const std::string & var_name,
                                                      unsigned int comp = 0) const;

  /**
   * Returns an old divergence from previous time step of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VectorVariableDivergence containing the old divergence of the coupled
   * variable
   * @see Kernel::_div_u_old
   */
  virtual const VectorVariableDivergence & coupledDivOld(const std::string & var_name,
                                                         unsigned int comp = 0) const;

  /**
   * Returns an old divergence from two time steps previous of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VectorVariableDivergence containing the older divergence of the coupled
   * variable
   * @see Kernel::_div_u_older
   */
  virtual const VectorVariableDivergence & coupledDivOlder(const std::string & var_name,
                                                           unsigned int comp = 0) const;

  /**
   * Returns second spatial derivatives of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableSecond containing the second derivative of the coupled variable
   * @see Kernel::second
   */
  virtual const VariableSecond & coupledSecond(const std::string & var_name,
                                               unsigned int comp = 0) const;

  /**
   * Returns an old second spatial derivatives from previous time step of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableSecond containing the old second derivative of the coupled
   * variable
   * @see Kernel::secondOld
   */
  virtual const VariableSecond & coupledSecondOld(const std::string & var_name,
                                                  unsigned int comp = 0) const;

  /**
   * Returns an old second derivative from two time steps previous of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableSecond containing the older second derivative of the coupled
   * variable
   * @see Kernel::secondOlder
   */
  virtual const VariableSecond & coupledSecondOlder(const std::string & var_name,
                                                    unsigned int comp = 0) const;

  /**
   * Returns second derivative of a coupled variable for the previous Newton iterate
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableSecond containing the second derivative of the coupled variable
   */
  virtual const VariableSecond & coupledSecondPreviousNL(const std::string & var_name,
                                                         unsigned int comp = 0) const;

  /**
   * Time derivative of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the time derivative of the coupled variable
   */
  virtual const VariableValue & coupledDot(const std::string & var_name,
                                           unsigned int comp = 0) const;

  /**
   * Returns the time derivatives for all of a coupled variable's components
   * @param var_name Name of coupled variable
   * @return Vector of VariableValue pointers for each component of \p var_name
   */
  std::vector<const VariableValue *> coupledDots(const std::string & var_name) const;

  /**
   * Second time derivative of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the second time derivative of the coupled
   * variable
   */
  virtual const VariableValue & coupledDotDot(const std::string & var_name,
                                              unsigned int comp = 0) const;

  /**
   * Old time derivative of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the old time derivative of the coupled variable
   */
  virtual const VariableValue & coupledDotOld(const std::string & var_name,
                                              unsigned int comp = 0) const;

  /**
   * Old second time derivative of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the old second time derivative of the coupled
   * variable
   */
  virtual const VariableValue & coupledDotDotOld(const std::string & var_name,
                                                 unsigned int comp = 0) const;

  /**
   * Time derivative of a coupled variable for ad simulations
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the time derivative of the coupled variable
   * @see Kernel::dot
   */
  const ADVariableValue & adCoupledDot(const std::string & var_name, unsigned int comp = 0) const;

  /**
   * Returns the time derivatives for all of a coupled variable's components for ad simulations
   * @param var_name Name of coupled variable
   * @return Vector of VariableValue pointers for each component of \p var_name
   */
  std::vector<const ADVariableValue *> adCoupledDots(const std::string & var_name) const;

  /**
   * Second time derivative of a coupled variable for ad simulations
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to an ADVariableValue containing the second time derivative of the coupled
   * variable
   */
  const ADVariableValue & adCoupledDotDot(const std::string & var_name,
                                          unsigned int comp = 0) const;

  /**
   * Time derivative of a vector coupled variable for ad simulations
   * @param var_name Name of vector coupled variable
   * @param comp Component number
   * @return Reference to a VectorVariableValue containing the time derivative of the coupled
   * variable
   * @see Kernel::dot
   */
  const ADVectorVariableValue & adCoupledVectorDot(const std::string & var_name,
                                                   unsigned int comp = 0) const;

  /**
   * Time derivative of a coupled vector variable
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VectorVariableValue containing the time derivative of the coupled
   * variable
   */
  virtual const VectorVariableValue & coupledVectorDot(const std::string & var_name,
                                                       unsigned int comp = 0) const;

  /**
   * Second time derivative of a coupled vector variable
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VectorVariableValue containing the time derivative of the coupled
   * variable
   */
  virtual const VectorVariableValue & coupledVectorDotDot(const std::string & var_name,
                                                          unsigned int comp = 0) const;

  /**
   * Old time derivative of a coupled vector variable
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VectorVariableValue containing the time derivative of the coupled
   * variable
   */
  virtual const VectorVariableValue & coupledVectorDotOld(const std::string & var_name,
                                                          unsigned int comp = 0) const;

  /**
   * Old second time derivative of a coupled vector variable
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VectorVariableValue containing the time derivative of the coupled
   * variable
   */
  virtual const VectorVariableValue & coupledVectorDotDotOld(const std::string & var_name,
                                                             unsigned int comp = 0) const;

  /**
   * Time derivative of a coupled vector variable with respect to the coefficients
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VariableValue containing the time derivative of the coupled
   * vector variable with respect to the coefficients
   */
  virtual const VariableValue & coupledVectorDotDu(const std::string & var_name,
                                                   unsigned int comp = 0) const;

  /**
   * Second time derivative of a coupled vector variable with respect to the coefficients
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VariableValue containing the time derivative of the coupled vector
   * variable with respect to the coefficients
   */
  virtual const VariableValue & coupledVectorDotDotDu(const std::string & var_name,
                                                      unsigned int comp = 0) const;

  /**
   * Time derivative of a coupled array variable
   * @param var_name Name of coupled array variable
   * @param comp Component number for vector of coupled array variables
   * @return Reference to a ArrayVariableValue containing the time derivative of the coupled
   * variable
   */
  virtual const ArrayVariableValue & coupledArrayDot(const std::string & var_name,
                                                     unsigned int comp = 0) const;

  /**
   * Second time derivative of a coupled array variable
   * @param var_name Name of coupled array variable
   * @param comp Component number for vector of coupled array variables
   * @return Reference to a ArrayVariableValue containing the time derivative of the coupled
   * variable
   */
  virtual const ArrayVariableValue & coupledArrayDotDot(const std::string & var_name,
                                                        unsigned int comp = 0) const;

  /**
   * Old time derivative of a coupled array variable
   * @param var_name Name of coupled array variable
   * @param comp Component number for vector of coupled array variables
   * @return Reference to a ArrayVariableValue containing the time derivative of the coupled
   * variable
   */
  virtual const ArrayVariableValue & coupledArrayDotOld(const std::string & var_name,
                                                        unsigned int comp = 0) const;

  /**
   * Old second time derivative of a coupled array variable
   * @param var_name Name of coupled array variable
   * @param comp Component number for vector of coupled array variables
   * @return Reference to a ArrayVariableValue containing the time derivative of the coupled
   * variable
   */
  virtual const ArrayVariableValue & coupledArrayDotDotOld(const std::string & var_name,
                                                           unsigned int comp = 0) const;

  /**
   * Time derivative of a coupled variable with respect to the coefficients
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the time derivative of the coupled variable
   * with respect to the coefficients
   */
  virtual const VariableValue & coupledDotDu(const std::string & var_name,
                                             unsigned int comp = 0) const;

  /**
   * Second time derivative of a coupled variable with respect to the coefficients
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the time derivative of the coupled variable
   * with respect to the coefficients
   */
  virtual const VariableValue & coupledDotDotDu(const std::string & var_name,
                                                unsigned int comp = 0) const;

  /**
   * Time derivative of a coupled array variable with respect to the coefficients
   * @param var_name Name of coupled array variable
   * @param comp Component number for vector of coupled array variables
   * @return Reference to a ArrayVariableValue containing the time derivative of the coupled
   * variable
   */
  const VariableValue & coupledArrayDotDu(const std::string & var_name,
                                          unsigned int comp = 0) const;

  /**
   * Returns nodal values of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the coupled variable
   */
  template <typename T>
  const T & coupledNodalValue(const std::string & var_name, unsigned int comp = 0) const;

  /**
   * Returns AD nodal values of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the coupled variable
   */
  template <typename T>
  const typename Moose::ADType<T>::type & adCoupledNodalValue(const std::string & var_name,
                                                              unsigned int comp = 0) const;

  /**
   * Returns an old nodal value from previous time step  of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the old value of the coupled variable
   */
  template <typename T>
  const T & coupledNodalValueOld(const std::string & var_name, unsigned int comp = 0) const;

  /**
   * Returns an old nodal value from two time steps previous of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the older value of the coupled variable
   */
  template <typename T>
  const T & coupledNodalValueOlder(const std::string & var_name, unsigned int comp = 0) const;

  /**
   * Returns nodal values of a coupled variable for previous Newton iterate
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the coupled variable
   */
  template <typename T>
  const T & coupledNodalValuePreviousNL(const std::string & var_name, unsigned int comp = 0) const;

  /**
   * Nodal values of time derivative of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the nodal values of time derivative of the
   * coupled variable
   */
  template <typename T>
  const T & coupledNodalDot(const std::string & var_name, unsigned int comp = 0) const;

  /**
   * Nodal values of second time derivative of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the nodal values of second time derivative of
   * the coupled variable
   */
  virtual const VariableValue & coupledNodalDotDot(const std::string & var_name,
                                                   unsigned int comp = 0) const;

  /**
   * Nodal values of old time derivative of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the nodal values of time derivative of the
   * coupled variable
   */
  virtual const VariableValue & coupledNodalDotOld(const std::string & var_name,
                                                   unsigned int comp = 0) const;

  /**
   * Nodal values of old second time derivative of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the nodal values of second time derivative of
   * the coupled variable
   */
  virtual const VariableValue & coupledNodalDotDotOld(const std::string & var_name,
                                                      unsigned int comp = 0) const;
  // coupled-dof-values-begin
  /**
   * Returns DoFs in the current solution vector of a coupled variable for the local element
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the DoFs of the coupled variable
   */
  virtual const VariableValue & coupledDofValues(const std::string & var_name,
                                                 unsigned int comp = 0) const;

  /**
   * Returns DoFs in the current solution vector of all of a coupled variable's components for the
   * local element
   * @param var_name Name of coupled variable
   * @return Vector of VariableValue pointers for each component of the coupled variable
   */
  std::vector<const VariableValue *> coupledAllDofValues(const std::string & var_name) const;

  /**
   * Returns DoFs in the old solution vector of a coupled variable for the local element
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the old DoFs of the coupled variable
   */
  virtual const VariableValue & coupledDofValuesOld(const std::string & var_name,
                                                    unsigned int comp = 0) const;

  /**
   * Returns DoFs in the old solution vector of all of a coupled variable's components for the local
   * element
   * @param var_name Name of coupled variable
   * @return Vector of VariableValue pointers for each compontnet of the coupled variable
   */
  std::vector<const VariableValue *> coupledAllDofValuesOld(const std::string & var_name) const;

  /**
   * Returns DoFs in the older solution vector of a coupled variable for the local element
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the older DoFs of the coupled variable
   */
  virtual const VariableValue & coupledDofValuesOlder(const std::string & var_name,
                                                      unsigned int comp = 0) const;

  /**
   * Returns DoFs in the older solution vector of all of a coupled variable's components for the
   * local element
   * @param var_name Name of coupled variable
   * @return Vector of VariableValue pointers for each component of the coupled variable
   */
  std::vector<const VariableValue *> coupledAllDofValuesOlder(const std::string & var_name) const;

  /**
   * Returns DoFs in the current solution vector of a coupled array variable for the local element
   * @param var_name Name of coupled array variable
   * @param comp Component number for vector of coupled array variables
   * @return Reference to a VariableValue for the DoFs of the coupled variable
   */
  virtual const ArrayVariableValue & coupledArrayDofValues(const std::string & var_name,
                                                           unsigned int comp = 0) const;
  // coupled-dof-values-end

  /**
   * Returns DOF value of a coupled variable for use in Automatic Differentiation
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to an ADVariableValue for the DoFs of the coupled variable
   */
  virtual const ADVariableValue & adCoupledDofValues(const std::string & var_name,
                                                     unsigned int comp = 0) const;

  /**
   * method that returns _zero to RESIDUAL computing objects and _ad_zero to JACOBIAN
   * computing objects
   */
  const ADVariableValue & adZeroValue() const;

  /**
   *  method that returns _grad_zero to RESIDUAL computing objects and _ad_grad_zero to
   * JACOBIAN computing objects
   */
  const ADVariableGradient & adZeroGradient() const;

  /**
   * Retrieve a zero second for automatic differentiation
   */
  const ADVariableSecond & adZeroSecond() const;

  /**
   * Returns zero value templated with automatic differentiation boolean
   * @return Reference to a const GenericVariableValue
   */
  template <bool is_ad>
  const GenericVariableValue<is_ad> & genericZeroValue();

  /**
   * Returns zero gradient templated with automatic differentiation boolean
   * @return Reference to a const GenericVariableValue
   */
  template <bool is_ad>
  const GenericVariableGradient<is_ad> & genericZeroGradient();

  /**
   * Returns zero second derivative templated with automatic differentiation boolean
   * @return Reference to a const GenericVariableValue
   */
  template <bool is_ad>
  const GenericVariableSecond<is_ad> & genericZeroSecond();

protected:
  // Reference to the interface's input parameters
  const InputParameters & _c_parameters;

  /// The name of the object this interface is part of
  const std::string & _c_name;
  /// The type of the object this interface is part of
  const std::string & _c_type;

  // Reference to FEProblemBase
  FEProblemBase & _c_fe_problem;

  /// Pointer to the system object if the moose object this is an interface for has one
  const SystemBase * const _c_sys;

  /// Coupled vars whose values we provide
  std::unordered_map<std::string, std::vector<MooseVariableFieldBase *>> _coupled_vars;

  /// Vector of all coupled variables
  std::vector<MooseVariableFieldBase *> _coupled_moose_vars;

  /// Vector of standard coupled variables
  std::vector<MooseVariable *> _coupled_standard_moose_vars;

  /// Vector of vector coupled variables
  std::vector<VectorMooseVariable *> _coupled_vector_moose_vars;

  /// Vector of array coupled variables
  std::vector<ArrayMooseVariable *> _coupled_array_moose_vars;

  /// Vector of standard finite volume coupled variables
  std::vector<MooseVariableFV<Real> *> _coupled_standard_fv_moose_vars;

  /// Vector of standard linear finite volume coupled variables
  std::vector<MooseLinearVariableFV<Real> *> _coupled_standard_linear_fv_moose_vars;

  /// map from new to deprecated variable names
  const std::unordered_map<std::string, std::string> & _new_to_deprecated_coupled_vars;

  /// True if we provide coupling to nodal values
  bool _c_nodal;

  /// True if implicit value is required
  bool _c_is_implicit;

  // Argument to allow element-to-nodal coupling
  const bool _c_allow_element_to_nodal_coupling;

  /// Thread ID of the thread using this object
  THREAD_ID _c_tid;

  /// Will hold the default value for optional coupled variables.
  mutable std::unordered_map<std::string, std::vector<std::unique_ptr<VariableValue>>>
      _default_value;

  /// Will hold the default value for optional coupled variables for automatic differentiation.
  mutable std::unordered_map<std::string, std::unique_ptr<MooseArray<ADReal>>> _ad_default_value;

  /// Will hold the default value for optional vector coupled variables.
  mutable std::unordered_map<std::string, std::unique_ptr<VectorVariableValue>>
      _default_vector_value;

  /// Will hold the default value for optional array coupled variables.
  mutable std::unordered_map<std::string, std::unique_ptr<ArrayVariableValue>> _default_array_value;

  /// Will hold the default value for optional vector coupled variables for automatic differentiation.
  mutable std::unordered_map<std::string, std::unique_ptr<MooseArray<ADRealVectorValue>>>
      _ad_default_vector_value;

  /**
   * This will always be zero because the default values for optionally coupled variables is always
   * constant and this is used for time derivative info
   */
  mutable VariableValue _default_value_zero;

  /// This will always be zero because the default values for optionally coupled variables is always constant
  mutable VariableGradient _default_gradient;

  /// This will always be zero because the default values for optionally coupled variables is always constant
  mutable MooseArray<ADRealVectorValue> _ad_default_gradient;

  /// This will always be zero because the default values for optionally coupled vector variables is always constant
  mutable MooseArray<ADRealTensorValue> _ad_default_vector_gradient;

  /// This will always be zero because the default values for optionally coupled variables is always constant
  mutable VariableSecond _default_second;

  /// This will always be zero because the default values for optionally coupled variables is always constant
  mutable MooseArray<ADRealTensorValue> _ad_default_second;

  /// Zero value of a variable
  const VariableValue & _zero;
  const VariablePhiValue & _phi_zero;
  const MooseArray<ADReal> & _ad_zero;

  /// Zero gradient of a variable
  const VariableGradient & _grad_zero;
  const MooseArray<ADRealVectorValue> & _ad_grad_zero;

  /// Zero gradient of trial function
  const VariablePhiGradient & _grad_phi_zero;

  /// Zero second derivative of a variable
  const VariableSecond & _second_zero;
  const MooseArray<ADRealTensorValue> & _ad_second_zero;
  /// Zero second derivative of a test function
  const VariablePhiSecond & _second_phi_zero;
  /// Zero value of a vector variable
  const VectorVariableValue & _vector_zero;
  /// Zero value of the curl of a vector variable
  const VectorVariableCurl & _vector_curl_zero;

  /**
   * This will always be zero because the default values for optionally coupled variables is always
   * constant and this is used for time derivative info
   */
  mutable VectorVariableValue _default_vector_value_zero;

  /// This will always be zero because the default values for optionally coupled variables is always constant
  mutable VectorVariableGradient _default_vector_gradient;

  /// This will always be zero because the default values for optionally coupled variables is always constant
  mutable VectorVariableCurl _default_vector_curl;

  /// This will always be zero because the default values for optionally coupled variables is always constant
  mutable VectorVariableDivergence _default_div;

  /**
   * This will always be zero because the default values for optionally coupled variables is always
   * constant and this is used for time derivative info
   */
  ArrayVariableValue _default_array_value_zero;

  /// This will always be zero because the default values for optionally coupled variables is always constant
  ArrayVariableGradient _default_array_gradient;

  /**
   * Check that the right kind of variable is being coupled in
   *
   * @param var_name The name of the coupled variable
   */
  bool
  checkVar(const std::string & var_name, unsigned int comp = 0, unsigned int comp_bound = 0) const;

private:
  /**
   * Generic helper method to get vector tag values based on tag ID
   */
  template <typename T>
  const typename OutputTools<T>::VariableValue &
  vectorTagValueHelper(const std::string & var_names, TagID tag, unsigned int index = 0) const;

  /**
   * Generic helper method to get vector tag values based on tag name
   */
  template <typename T>
  const typename OutputTools<T>::VariableValue & vectorTagValueHelper(const std::string & var_names,
                                                                      const std::string & tag_name,
                                                                      unsigned int index = 0) const;

  /**
   * Generic helper method to get vector tag degree of freedom values based on tag ID
   */
  template <typename T>
  const typename OutputTools<T>::VariableValue &
  vectorTagDofValueHelper(const std::string & var_name, TagID tag, unsigned int comp = 0) const;

  /**
   * Generic helper method to get vector tag degree of freedom values based on tag name
   */
  template <typename T>
  const typename OutputTools<T>::VariableValue & vectorTagDofValueHelper(
      const std::string & var_name, const std::string & tag_name, unsigned int comp = 0) const;

  /**
   * Method that may request additional solution states from the variable's system depending on the
   * value of \p tag_name. E.g. if the tag name corresponds to old or older variable solution
   * values, then we must request more states
   */
  template <typename T>
  void
  requestStates(const std::string & var_name, const TagName & tag_name, const unsigned int comp);

  enum class FuncAge
  {
    Curr,
    Old,
    Older,
  };

  enum class VarType
  {
    Ignore,
    Gradient,
    Second,
    GradientDot,
    Dot,
  };

  void checkFuncType(const std::string var_name, VarType t, FuncAge age) const;

protected:
  /**
   * Deprecated method. Use \p getFieldVar instead
   * Extract pointer to a base coupled field variable. Could be either a finite volume or finite
   * element variable
   * @param var_name Name of parameter desired
   * @param comp Component number of multiple coupled variables
   * @return Pointer to the desired variable
   */
  const MooseVariableFieldBase * getFEVar(const std::string & var_name, unsigned int comp) const;

  /*
   * Extract pointer to a base coupled field variable. Could be either a finite volume or finite
   * element variable
   * @param var_name Name of parameter desired
   * @param comp Component number of multiple coupled variables
   * @return Pointer to the desired variable
   */
  const MooseVariableFieldBase * getFieldVar(const std::string & var_name, unsigned int comp) const;

  /*
   * Extract pointer to a base coupled field variable. Could be either a finite volume or finite
   * element variable
   * @param var_name Name of variable desired
   * @param comp Component number of multiple coupled variables
   * @return Pointer to the desired variable
   */
  MooseVariableFieldBase * getFieldVar(const std::string & var_name, unsigned int comp);

  /**
   * Helper that that be used to retrieve a variable of arbitrary type \p T
   */
  template <typename T>
  const T * getVarHelper(const std::string & var_name, unsigned int comp) const;

  /**
   * Helper that can be used to retrieve a variable of arbitrary type \p T
   */
  template <typename T>
  T * getVarHelper(const std::string & var_name, unsigned int comp);

  /**
   * Extract pointer to a coupled variable
   * @param var_name Name of parameter desired
   * @param comp Component number of multiple coupled variables
   * @return Pointer to the desired variable
   */
  MooseVariable * getVar(const std::string & var_name, unsigned int comp);

  /**
   * Extract pointer to a coupled vector variable
   * @param var_name Name of parameter desired
   * @param comp Component number of multiple coupled variables
   * @return Pointer to the desired variable
   */
  VectorMooseVariable * getVectorVar(const std::string & var_name, unsigned int comp);

  /**
   * Extract pointer to a coupled array variable
   * @param var_name Name of parameter desired
   * @param comp Component number of multiple coupled variables
   * @return Pointer to the desired variable
   */
  ArrayMooseVariable * getArrayVar(const std::string & var_name, unsigned int comp);

  /**
   * Extract pointer to a coupled variable
   * @param var_name Name of parameter desired
   * @param comp Component number of multiple coupled variables
   * @return Pointer to the desired variable
   */
  const MooseVariable * getVar(const std::string & var_name, unsigned int comp) const;

  /**
   * Extract pointer to a coupled vector variable
   * @param var_name Name of parameter desired
   * @param comp Component number of multiple coupled variables
   * @return Pointer to the desired variable
   */
  const VectorMooseVariable * getVectorVar(const std::string & var_name, unsigned int comp) const;

  /**
   * Extract pointer to a coupled array variable
   * @param var_name Name of parameter desired
   * @param comp Component number of multiple coupled variables
   * @return Pointer to the desired variable
   */
  const ArrayMooseVariable * getArrayVar(const std::string & var_name, unsigned int comp) const;

  /**
   * Checks to make sure that the current Executioner has set "_is_transient" when old/older values
   * are coupled in.
   * @param name the name of the variable
   * @param fn_name The name of the function that called this method - used in the error message
   */
  void validateExecutionerType(const std::string & name, const std::string & fn_name) const;

  template <typename T, typename Func>
  std::vector<T> coupledVectorHelper(const std::string & var_name, const Func & func) const
  {
    const auto components = coupledComponents(var_name);
    std::vector<T> vals(components);
    for (MooseIndex(components) comp = 0; comp < components; ++comp)
      vals[comp] = func(comp);
    return vals;
  }

  /// Whether or not this object is a "neighbor" object: ie all of it's coupled values should be neighbor values
  bool _coupleable_neighbor;

public:
  /**
   * Helper method to return (and insert if necessary) the default value for Automatic
   * Differentiation for an uncoupled variable.
   * @param var_name the name of the variable for which to retrieve a default value
   * @return VariableValue * a pointer to the associated VariableValue.
   */
  const ADVariableValue * getADDefaultValue(const std::string & var_name) const;

  /**
   * Helper method to return (and insert if necessary) the default vector value for Automatic
   * Differentiation for an uncoupled variable.
   * @param var_name the name of the vector variable for which to retrieve a default value
   * @return VectorVariableValue * a pointer to the associated VectorVariableValue.
   */
  const ADVectorVariableValue * getADDefaultVectorValue(const std::string & var_name) const;

  /**
   * Helper method to return (and insert if necessary) the default gradient for Automatic
   * Differentiation for an uncoupled variable.
   * @param var_name the name of the variable for which to retrieve a default gradient
   * @return VariableGradient * a pointer to the associated VariableGradient.
   */
  const ADVariableGradient & getADDefaultGradient() const;

  /**
   * Helper method to return (and insert if necessary) the default gradient for Automatic
   * Differentiation for an uncoupled vector variable.
   * @param var_name the name of the vector variable for which to retrieve a default gradient
   * @return VariableGradient * a pointer to the associated VectorVariableGradient.
   */
  const ADVectorVariableGradient & getADDefaultVectorGradient() const;

  /**
   * Helper method to return (and insert if necessary) the default second derivatives for Automatic
   * Differentiation for an uncoupled variable.
   * @param var_name the name of the variable for which to retrieve a default second derivative
   * @return VariableSecond * a pointer to the associated VariableSecond.
   */
  const ADVariableSecond & getADDefaultSecond() const;

private:
  /**
   * Helper method to return (and insert if necessary) the default value
   * for an uncoupled variable.
   * @param var_name the name of the variable for which to retrieve a default value
   * @return a pointer to the associated VariableValue.
   */
  const VariableValue * getDefaultValue(const std::string & var_name, unsigned int comp) const;

  /**
   * Helper method to return (and insert if necessary) the default value
   * for an uncoupled vector variable.
   * @param var_name the name of the vector variable for which to retrieve a default value
   * @return a pointer to the associated VectorVariableValue.
   */
  const VectorVariableValue * getDefaultVectorValue(const std::string & var_name) const;

  /**
   * Helper method to return (and insert if necessary) the default value
   * for an uncoupled array variable.
   * @param var_name the name of the vector variable for which to retrieve a default value
   * @return a pointer to the associated VectorVariableValue.
   */
  const ArrayVariableValue * getDefaultArrayValue(const std::string & var_name) const;

  /**
   * Get nodal default value
   */
  template <typename T>
  const T & getDefaultNodalValue(const std::string & var_name, unsigned int comp = 0) const;

  template <typename T>
  const Moose::Functor<T> & getDefaultFunctor(const std::string & var_name) const;

  /// Maximum qps for any element in this system
  unsigned int _coupleable_max_qps;

  /// Unique indices for optionally coupled vars that weren't provided
  std::unordered_map<std::string, std::vector<unsigned int>> _optional_var_index;

  /// Scalar variables coupled into this object (for error checking)
  std::unordered_map<std::string, std::vector<MooseVariableScalar *>> _c_coupled_scalar_vars;

  std::set<TagID> _fe_coupleable_vector_tags;

  std::set<TagID> _fe_coupleable_matrix_tags;

  /// Whether the MooseObject is a finite volume object
  const bool _is_fv;

  const MooseObject * const _obj;

  /// vector tag names for which we need to request older solution states from the system
  const std::set<std::string> _older_state_tags = {Moose::OLD_SOLUTION_TAG,
                                                   Moose::OLDER_SOLUTION_TAG};

  /// keep a set of allocated writable variable references to make sure only one object can obtain them per thread
  std::vector<std::set<MooseWritableVariable *>> _writable_coupled_variables;
};

template <typename T>
T *
Coupleable::getVarHelper(const std::string & var_name_in, unsigned int comp)
{
  const auto var_name = _c_parameters.checkForRename(var_name_in);
  auto name_to_use = var_name;

  // First check for supplied name
  if (!checkVar(var_name, comp, 0))
  {
    // See if there is an associated deprecated name that the user may have used instead
    auto it = _new_to_deprecated_coupled_vars.find(var_name);
    if (it == _new_to_deprecated_coupled_vars.end())
      return nullptr;
    else
    {
      auto deprecated_name = it->second;
      if (checkVar(deprecated_name, comp, 0))
        name_to_use = deprecated_name;
      else
        return nullptr;
    }
  }

  auto coupled_vars_it = _coupled_vars.find(name_to_use);

  mooseAssert(coupled_vars_it != _coupled_vars.end(),
              "Trying to get a coupled var " << name_to_use << " that doesn't exist");

  if (auto coupled_var = dynamic_cast<T *>(coupled_vars_it->second[comp]))
    return coupled_var;
  else
  {
    for (auto & var : _coupled_standard_moose_vars)
      if (var->name() == name_to_use)
        mooseError("The named variable is a standard variable, try a "
                   "'coupled[Value/Gradient/Dot/etc]...' function instead");
    for (auto & var : _coupled_vector_moose_vars)
      if (var->name() == name_to_use)
        mooseError("The named variable is a vector variable, try a "
                   "'coupledVector[Value/Gradient/Dot/etc]...' function instead");
    for (auto & var : _coupled_array_moose_vars)
      if (var->name() == name_to_use)
        mooseError("The named variable is an array variable, try a "
                   "'coupledArray[Value/Gradient/Dot/etc]...' function instead");
    for (auto & var : _coupled_standard_fv_moose_vars)
      if (var->name() == name_to_use)
        mooseError("The named variable is a finite volume variable, which the coupled[...] routine "
                   "used does not support. Try using the functor system routines instead.");
    mooseError(
        "Variable '", name_to_use, "' is of a different C++ type than you tried to fetch it as.");
  }
}

template <typename T>
const T *
Coupleable::getVarHelper(const std::string & var_name, unsigned int comp) const
{
  return const_cast<Coupleable *>(this)->getVarHelper<T>(var_name, comp);
}
