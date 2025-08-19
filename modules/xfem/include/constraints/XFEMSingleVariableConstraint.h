//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "GenericElemElemConstraint.h"
#include "MooseMesh.h"

// Forward Declarations
class XFEM;
class Function;

template <bool is_ad>
class XFEMSingleVariableConstraintTempl : public GenericElemElemConstraint<is_ad>
{
public:
  static InputParameters validParams();

  XFEMSingleVariableConstraintTempl(const InputParameters & parameters);
  virtual ~XFEMSingleVariableConstraintTempl();

protected:
  virtual void reinitConstraintQuadrature(const ElementPairInfo & element_pair_info) override;

  virtual GenericReal<is_ad> computeQpResidual(Moose::DGResidualType type) override;

  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;

  /// Vector normal to the internal interface
  Point _interface_normal;

  /// Stabilization parameter in Nitsche's formulation and penalty factor in the
  /// Penalty Method
  Real _alpha;

  /// Change in variable value at the interface
  const Function & _jump;

  /// Change in flux of variable value at the interface
  const Function & _jump_flux;

  /// Use penalty formulation
  bool _use_penalty;

  /// Pointer to the XFEM controller object
  std::shared_ptr<XFEM> _xfem;

  // Declare usage of base class members
  usingGenericElemElemConstraint;
};

typedef XFEMSingleVariableConstraintTempl<false> XFEMSingleVariableConstraint;
typedef XFEMSingleVariableConstraintTempl<true> ADXFEMSingleVariableConstraint;
