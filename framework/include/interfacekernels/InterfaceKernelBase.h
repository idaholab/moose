//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// local includes
#include "MooseArray.h"
#include "NeighborResidualObject.h"
#include "BoundaryRestrictable.h"
#include "NeighborCoupleableMooseVariableDependencyIntermediateInterface.h"
#include "TwoMaterialPropertyInterface.h"
#include "ElementIDInterface.h"

/**
 * InterfaceKernelBase is the base class for all InterfaceKernel type classes.
 */

class InterfaceKernelBase : public NeighborResidualObject,
                            public BoundaryRestrictable,
                            public NeighborCoupleableMooseVariableDependencyIntermediateInterface,
                            public TwoMaterialPropertyInterface,
                            public ElementIDInterface
{
public:
  static InputParameters validParams();

  InterfaceKernelBase(const InputParameters & parameters);

  /// The neighbor variable number that this interface kernel operates on
  virtual const MooseVariableFEBase & neighborVariable() const = 0;

  /**
   * Using the passed DGResidual type, selects the correct test function space and residual block,
   * and then calls computeQpResidual
   */
  virtual void computeElemNeighResidual(Moose::DGResidualType type) = 0;

  /**
   * Using the passed DGJacobian type, selects the correct test function and trial function spaces
   * and
   * jacobian block, and then calls computeQpJacobian
   */
  virtual void computeElemNeighJacobian(Moose::DGJacobianType type) = 0;

  /**
   * Using the passed DGJacobian type, selects the correct test function and trial function spaces
   * and
   * jacobian block, and then calls computeQpOffDiagJacobian with the passed jvar
   */
  virtual void computeOffDiagElemNeighJacobian(Moose::DGJacobianType type, unsigned int jvar) = 0;

  /// Selects the correct Jacobian type and routine to call for the primary variable jacobian
  virtual void computeElementOffDiagJacobian(unsigned int jvar) = 0;

  /// Selects the correct Jacobian type and routine to call for the secondary variable jacobian
  virtual void computeNeighborOffDiagJacobian(unsigned int jvar) = 0;

  void prepareShapes(unsigned int var_num) override final;

protected:
  /// Compute jacobians at quadrature points
  virtual Real computeQpJacobian(Moose::DGJacobianType /*type*/) { return 0; }

  /// compute off-diagonal jacobian components at quadrature points
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType /*type*/, unsigned int /*jvar*/)
  {
    return 0;
  }

  /// The volume of the current neighbor
  const Real & getNeighborElemVolume();

  /// Pointer reference to the current element
  const Elem * const & _current_elem;

  /// The volume (or length) of the current element
  const Real & _current_elem_volume;

  /// The neighboring element
  const Elem * const & _neighbor_elem;

  /// The neighboring element volume
  const Real & _neighbor_elem_volume;

  /// Current side
  const unsigned int & _current_side;

  /// Current side element
  const Elem * const & _current_side_elem;

  /// The volume (or length) of the current side
  const Real & _current_side_volume;

  /// Coordinate system
  const Moose::CoordinateSystemType & _coord_sys;

  /// Current quadrature point
  unsigned int _qp;

  /// Array that holds element quadrature point coordinates
  const MooseArray<Point> & _q_point;

  /// Quadrature rule
  const QBase * const & _qrule;

  /// Elemtn Jacobian/quadrature weight
  const MooseArray<Real> & _JxW;

  /// Coordinate transformation value; relevant in axisymmetric simulations for example
  const MooseArray<Real> & _coord;

  /// Index for test and trial functions
  unsigned int _i, _j;

  /** MultiMooseEnum specifying whether residual save-in
   * aux variables correspond to primary or secondary side
   */
  MultiMooseEnum _save_in_var_side;

  /** The names of the aux variables that will be used to save-in residuals
   * (includes both primary and secondary variable names)
   */
  std::vector<AuxVariableName> _save_in_strings;

  /// Whether there are primary residual aux variables
  bool _has_primary_residuals_saved_in;

  /// The aux variables to save the primary residual contributions to
  std::vector<MooseVariableFEBase *> _primary_save_in_residual_variables;

  /// Whether there are secondary residual aux variables
  bool _has_secondary_residuals_saved_in;

  /// The aux variables to save the secondary contributions to
  std::vector<MooseVariableFEBase *> _secondary_save_in_residual_variables;

  /** MultiMooseEnum specifying whether jacobian save-in
   * aux variables correspond to primary or secondary side
   */
  MultiMooseEnum _diag_save_in_var_side;

  /** The names of the aux variables that will be used to save-in jacobians
   * (includes both primary and secondary variable names)
   */
  std::vector<AuxVariableName> _diag_save_in_strings;

  /// Whether there are primary jacobian aux variables
  bool _has_primary_jacobians_saved_in;

  /// The aux variables to save the diagonal Jacobian contributions of the primary variables to
  std::vector<MooseVariableFEBase *> _primary_save_in_jacobian_variables;

  /// Whether there are secondary jacobian aux variables
  bool _has_secondary_jacobians_saved_in;

  /// The aux variables to save the diagonal Jacobian contributions of the secondary variables to
  std::vector<MooseVariableFEBase *> _secondary_save_in_jacobian_variables;

  /// Mutex that prevents multiple threads from saving into the residual aux_var at the same time
  static Threads::spin_mutex _resid_vars_mutex;

  /// Mutex that prevents multiple threads from saving into the jacobian aux_var at the same time
  static Threads::spin_mutex _jacoby_vars_mutex;
};
