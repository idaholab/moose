//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "DiracKernelInfo.h"
#include "MooseVariableInterface.h"
#include "DiracKernelBase.h"

// forward declarations
#define TemplateVariableValue typename OutputTools<T>::VariableValue
#define TemplateVariableGradient typename OutputTools<T>::VariableGradient
#define TemplateVariablePhiValue typename OutputTools<T>::VariablePhiValue
#define TemplateVariablePhiGradient typename OutputTools<T>::VariablePhiGradient
#define TemplateVariableTestValue typename OutputTools<T>::VariableTestValue
#define TemplateVariableTestGradient typename OutputTools<T>::VariableTestGradient

template <typename>
class DiracKernelTempl;

using DiracKernel = DiracKernelTempl<Real>;
using VectorDiracKernel = DiracKernelTempl<RealVectorValue>;

/**
 * A DiracKernel is used when you need to add contributions to the residual by means of
 * multiplying some number by the shape functions on an element and adding the value into
 * the residual vector at the places associated with that shape function.
 *
 * This is common in point sources / sinks and various other algorithms.
 */
template <typename T>
class DiracKernelTempl : public DiracKernelBase, public MooseVariableInterface<T>
{
public:
  static InputParameters validParams();

  DiracKernelTempl(const InputParameters & parameters);

  /**
   * Computes the residual for the current element.
   */
  virtual void computeResidual() override;

  /**
   * Computes the jacobian for the current element.
   */
  virtual void computeJacobian() override;

  /**
   * This gets called by computeOffDiagJacobian() at each quadrature point.
   */
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /**
   * Computes the off-diagonal Jacobian for variable jvar.
   */
  virtual void computeOffDiagJacobian(unsigned int jvar) override;

  virtual const MooseVariableField<T> & variable() const override { return _var; }

  /**
   * This is where the DiracKernel should call addPoint() for each point it needs to have a
   * value distributed at.
   */
  virtual void addPoints() override;

protected:
  /**
   * This is the virtual that derived classes should override for computing the residual.
   */
  virtual Real computeQpResidual() override;

  /**
   * This is the virtual that derived classes should override for computing the Jacobian.
   */
  virtual Real computeQpJacobian() override;

  /**
   * Returns the user-assigned ID of the current Dirac point if it
   * exits, and libMesh::invalid_uint otherwise.  Can be used e.g. in
   * the computeQpResidual() function to determine the cached ID of
   * the current point, in case this information is relevant.
   */
  unsigned currentPointCachedID();

  /// Variable this kernel acts on
  MooseVariableField<T> & _var;

  ///< Current element
  const Elem * const & _current_elem;

  // shape functions

  /// Values of shape functions at QPs
  const TemplateVariablePhiValue & _phi;
  /// Gradients of shape functions at QPs
  const TemplateVariablePhiGradient & _grad_phi;

  // test functions

  /// Values of test functions at QPs
  const TemplateVariableTestValue & _test;
  /// Gradients of test functions at QPs
  const TemplateVariableTestGradient & _grad_test;

  /// Holds the solution at current quadrature points
  const TemplateVariableValue & _u;
  /// Holds the solution gradient at the current quadrature points
  const TemplateVariableGradient & _grad_u;

  /// drop duplicate points or consider them in residual and Jacobian
  const bool _drop_duplicate_points;

  // @{ Point-not-found behavior
  enum class PointNotFoundBehavior
  {
    ERROR,
    WARNING,
    IGNORE
  };
  const PointNotFoundBehavior _point_not_found_behavior;
  // @}

private:
};
