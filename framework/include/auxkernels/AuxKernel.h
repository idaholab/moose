//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernelBase.h"
#include "MooseVariableInterface.h"

// forward declarations
template <typename ComputeValueType>
class AuxKernelTempl;

typedef AuxKernelTempl<Real> AuxKernel;
typedef AuxKernelTempl<RealVectorValue> VectorAuxKernel;
typedef AuxKernelTempl<RealEigenVector> ArrayAuxKernel;

/**
 * Base class for creating new auxiliary kernels and auxiliary boundary conditions.
 */
template <typename ComputeValueType>
class AuxKernelTempl : public AuxKernelBase, public MooseVariableInterface<ComputeValueType>
{
public:
  static InputParameters validParams();

  AuxKernelTempl(const InputParameters & parameters);

  /**
   * Computes the value and stores it in the solution vector
   */
  virtual void compute() override;

  /**
   * Nodal or elemental kernel?
   * @return true if this is a nodal kernel, otherwise false
   */
  bool isNodal() const { return _nodal; }

  /**
   * @return whether this is a mortar auxiliary kernel
   */
  bool isMortar();

  /**
   * Get a reference to a variable this kernel is action on
   * @return reference to a variable this kernel is action on
   */
  MooseVariableField<ComputeValueType> & variable() { return _var; }

  /**
   * Override functions from MaterialPropertyInterface for error checking
   */
  template <typename T>
  const MaterialProperty<T> & getMaterialProperty(const std::string & name);
  template <typename T, bool is_ad>
  const GenericMaterialProperty<T, is_ad> & getGenericMaterialProperty(const std::string & name);
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOld(const std::string & name);
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOlder(const std::string & name);

  /**
   * Insert the just computed values into the auxiliary solution vector
   */
  void insert();

  /**
   * Determines whether we're a coincident lower-d calculation
   */
  void determineWhetherCoincidentLowerDCalc();

protected:
  /**
   * Compute and return the value of the aux variable.
   */
  virtual ComputeValueType computeValue() = 0;

  virtual const VariableValue & coupledDot(const std::string & var_name,
                                           unsigned int comp = 0) const override;

  virtual const VariableValue & coupledDotDu(const std::string & var_name,
                                             unsigned int comp = 0) const override;

  /// This callback is used for AuxKernelTempls that need to perform a per-element calculation
  virtual void precalculateValue() {}

  /**
   * Retrieves the old value of the variable that this AuxKernel operates on.
   *
   * Store this as a _reference_ in the constructor.
   */
  const typename OutputTools<ComputeValueType>::VariableValue & uOld() const;

  /**
   * Retrieves the older value of the variable that this AuxKernel operates on.
   *
   * Store this as a _reference_ in the constructor.
   */
  const typename OutputTools<ComputeValueType>::VariableValue & uOlder() const;

  /// This is a regular kernel so we cast to a regular MooseVariable, hides base _var
  MooseVariableField<ComputeValueType> & _var;

  /// Flag indicating if the AuxKernel is nodal
  const bool _nodal;

  /// Holds the solution at current quadrature points
  const typename OutputTools<ComputeValueType>::VariableValue & _u;

  /// Holds the the test functions
  const typename OutputTools<ComputeValueType>::VariableTestValue & _test;

  /// Active quadrature points
  const MooseArray<Point> & _q_point;
  /// Quadrature rule being used
  const QBase * const & _qrule;
  /// Transformed Jacobian weights
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;

  /// Current element (valid only for elemental kernels)
  const Elem * const & _current_elem;
  /// current side of the current element
  const unsigned int & _current_side;

  /// Volume of the current element
  const Real & _current_elem_volume;
  /// Volume of the current side
  const Real & _current_side_volume;

  /// Current node (valid only for nodal kernels)
  const Node * const & _current_node;

  /// The current boundary ID
  const BoundaryID & _current_boundary_id;

  /// reference to the solution vector of auxiliary system
  NumericVector<Number> & _solution;

  /// The current lower dimensional element
  const Elem * const & _current_lower_d_elem;

  /// Quadrature point index
  unsigned int _qp;

  /// number of shape functions for the finite element type and current DofObject
  unsigned int _n_shapes;

  typedef typename Moose::DOFType<ComputeValueType>::type OutputData;

  /// for holding local load
  DenseVector<OutputData> _local_re;
  /// for holding local solution
  DenseVector<OutputData> _local_sol;
  /// for holding local mass matrix
  DenseMatrix<Number> _local_ke;

  /// Whether we are computing for a lower dimensional variable using boundary restriction, e.g. a
  /// variable whose block restriction is coincident with a higher-dimensional boundary face
  std::optional<bool> _coincident_lower_d_calc;

  using MooseVariableInterface<ComputeValueType>::mooseVariableBase;

private:
  /**
   * Currently only used when the auxiliary variable is a finite volume variable, this helps call
   * through to the variable's \p setDofValue method. This helper is necessary because \p
   * MooseVariableField::setDofValue expects a \p Real even when a variable is a vector variable, so
   * we cannot simply pass through to that method with the result of \p computeValue when \p
   * ComputeValueType is \p RealVectorValue
   */
  void setDofValueHelper(const ComputeValueType & dof_value);
};

template <typename ComputeValueType>
template <typename T>
const MaterialProperty<T> &
AuxKernelTempl<ComputeValueType>::getMaterialProperty(const std::string & name)
{
  if (isNodal())
    mooseError("Nodal AuxKernel '",
               AuxKernelTempl::name(),
               "' attempted to reference material property '",
               name,
               "'\nConsider using an elemental auxiliary variable for '",
               _var.name(),
               "'.");

  return MaterialPropertyInterface::getMaterialProperty<T>(name);
}

template <typename ComputeValueType>
template <typename T, bool is_ad>
const GenericMaterialProperty<T, is_ad> &
AuxKernelTempl<ComputeValueType>::getGenericMaterialProperty(const std::string & name)
{
  if (isNodal())
    mooseError("Nodal AuxKernel '",
               AuxKernelTempl::name(),
               "' attempted to reference material property '",
               name,
               "'\nConsider using an elemental auxiliary variable for '",
               _var.name(),
               "'.");

  return MaterialPropertyInterface::getGenericMaterialProperty<T, is_ad>(name);
}

template <typename ComputeValueType>
template <typename T>
const MaterialProperty<T> &
AuxKernelTempl<ComputeValueType>::getMaterialPropertyOld(const std::string & name)
{
  if (isNodal())
    mooseError("Nodal AuxKernel '",
               AuxKernelTempl::name(),
               "' attempted to reference material property '",
               name,
               "'\nConsider using an elemental auxiliary variable for '",
               _var.name(),
               "'.");

  return MaterialPropertyInterface::getMaterialPropertyOld<T>(name);
}

template <typename ComputeValueType>
template <typename T>
const MaterialProperty<T> &
AuxKernelTempl<ComputeValueType>::getMaterialPropertyOlder(const std::string & name)
{
  if (isNodal())
    mooseError("Nodal AuxKernel '",
               AuxKernelTempl::name(),
               "' attempted to reference material property '",
               name,
               "'\nConsider using an elemental auxiliary variable for '",
               _var.name(),
               "'.");

  return MaterialPropertyInterface::getMaterialPropertyOlder<T>(name);
}

// Declare all the specializations, as the template specialization declaration below must know
template <>
void AuxKernelTempl<Real>::setDofValueHelper(const Real & value);
template <>
void AuxKernelTempl<RealVectorValue>::setDofValueHelper(const RealVectorValue &);
template <>
void AuxKernelTempl<RealEigenVector>::compute();

// Prevent implicit instantiation in other translation units where these classes are used
extern template class AuxKernelTempl<Real>;
extern template class AuxKernelTempl<RealVectorValue>;
extern template class AuxKernelTempl<RealEigenVector>;
