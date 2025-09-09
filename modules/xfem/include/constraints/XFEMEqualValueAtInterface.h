//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericElemElemConstraint.h"

// Forward Declarations
class XFEM;

template <bool is_ad>
class XFEMEqualValueAtInterfaceTempl : public GenericElemElemConstraint<is_ad>
{
public:
  static InputParameters validParams();

  XFEMEqualValueAtInterfaceTempl(const InputParameters & parameters);
  virtual ~XFEMEqualValueAtInterfaceTempl();

protected:
  virtual void reinitConstraintQuadrature(const ElementPairInfo & element_pair_info) override;

  virtual GenericReal<is_ad> computeQpResidual(Moose::DGResidualType type) override;

  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;

  // Penalty parameter in penalty formulation
  Real _alpha;

  /// Value at the interface
  Real _value;

  /// Pointer to the XFEM controller object
  std::shared_ptr<XFEM> _xfem;

  // Declare usage of base class members
  usingGenericElemElemConstraint;
};

typedef XFEMEqualValueAtInterfaceTempl<false> XFEMEqualValueAtInterface;
typedef XFEMEqualValueAtInterfaceTempl<true> ADXFEMEqualValueAtInterface;
